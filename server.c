/// @file

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include <errno.h>
#include <pthread.h>
#include <sys/queue.h>

#include "include/args.h"
#include "include/log.h"
#include "include/socket.h"
#include "include/ctmp.h"

struct client_entry {
	int fd;  ///< socket file descriptor
	bool open;  ///< is the connection open?
	TAILQ_ENTRY(client_entry) entries;  ///< prev + next pointers for client queue
};
TAILQ_HEAD(client_queue, client_entry);  ///< define client_queue as a doubly linked tail queue
pthread_mutex_t client_lock = PTHREAD_MUTEX_INITIALIZER;
struct client_queue client_queue_head;

struct msg_entry {
	struct ctmp_msg *msg;  ///< CTMP message structure to broadcast
	TAILQ_ENTRY(msg_entry) entries;  ///< prev + next pointers for message queue
};
TAILQ_HEAD(msg_queue, msg_entry);  ///< define msg_queue as a doubly linked tail queue
pthread_mutex_t msg_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t msg_cond = PTHREAD_COND_INITIALIZER;
struct msg_queue msg_queue_head;

struct args init_args;

/**
 * @brief Broadcast a CTMP message to all connected receivers
 * @details Send a given message to all receivers in the client queue, removing
 * entries relating to closed connections
 * @param msg message to broadcast
 */
void broadcast(struct ctmp_msg *msg)
{
	struct client_entry *current = NULL, *next = NULL;
	ssize_t bytes_sent = 0;

	pthread_mutex_lock(&client_lock);
	current = TAILQ_FIRST(&client_queue_head);
	pthread_mutex_unlock(&client_lock);

	while (current) {
		bytes_sent = send_ctmp_msg(current->fd, msg);
		if (bytes_sent < 0) {
			current->open = false;
		}

		pthread_mutex_lock(&client_lock);
		next = TAILQ_NEXT(current, entries);
		pthread_mutex_unlock(&client_lock);

		if (!current->open) {
			pr_debug("removing receiver connection (fd %d)\n",
					current->fd);
			close(current->fd);
			pthread_mutex_lock(&client_lock);
			TAILQ_REMOVE(&client_queue_head, current, entries);
			pthread_mutex_unlock(&client_lock);
			free(current);
		}

		current = next;
	}
}

/**
 * @brief Run source server
 * @details Accept a single client connection and parse messages from it,
 * broadcasting to receivers when valid
 */
void src_server()
{
	int src_socket;
	struct server_socket *src_server = NULL;
	struct ctmp_msg *current_msg = NULL;
	struct ctmp_msg *(*parse_func)(int) = NULL;   ///< CTMP message parsing function
	struct msg_entry *new_msg_entry = NULL;

	src_server = server_create(SRC_PORT);
	if (!src_server) {
		pr_err("error setting up server on port %d\n", SRC_PORT);
		exit(EXIT_FAILURE);
	}

	/* check which protocol version to use */
	if (init_args.extended) {
		parse_func = &parse_ctmp_msg_extended;
	} else {
		parse_func = &parse_ctmp_msg;
	}

	while (1) {
		char test_buffer;

		src_socket = server_accept(src_server->fd, src_server->addr);
		if (src_socket < 0) {
			pr_err("error accepting connection to port %d\n", SRC_PORT);
			exit(-src_socket);
		}

		/* keep parsing messages while the connection is open
		 *
		 * based on: http://stefan.buettcher.org/cs/conn_closed.html
		 * MSG_PEEK: return data without removing it from the queue
		 * MSG_DONTWAIT: enable nonblocking */
		while (recv(src_socket, &test_buffer, sizeof(test_buffer),
					MSG_PEEK | MSG_DONTWAIT) != 0) {
			current_msg = parse_func(src_socket);
			if (current_msg) {
				/* add message to queue */
				new_msg_entry = malloc(sizeof(struct msg_entry));
				if (!new_msg_entry) {
					perror("malloc");
					exit(errno);
				}

				new_msg_entry->msg = current_msg;
				pthread_mutex_lock(&msg_lock);
				/* add message to queue */
				TAILQ_INSERT_TAIL(&msg_queue_head, new_msg_entry, entries);
				/* signal that there is a message to broadcast */
				pthread_cond_signal(&msg_cond);
				pthread_mutex_unlock(&msg_lock);
			}
		}

		pr_debug("closing src connection...\n");
		close(src_socket);
	}
}

/**
 * @brief Run destination server
 * @details Accept client connections and update the shared queue of client file
 * descriptors
 */
void *dst_server()
{
	int new_fd;
	struct server_socket *dst_server = NULL;
	struct client_entry *new_client_entry = NULL;

	dst_server = server_create(DST_PORT);
	if (!dst_server) {
		pr_err("error setting up server on port %d\n", DST_PORT);
		exit(EXIT_FAILURE);
	}

	while (1) {
		new_fd = server_accept(dst_server->fd, dst_server->addr);
		if (new_fd < 0) {
			pr_err("error accepting connection to port %d\n", DST_PORT);
			/* retry */
			continue;
		}

		new_client_entry = malloc(sizeof(struct client_entry));
		if (!new_client_entry) {
			perror("malloc");
			exit(errno);
		}

		new_client_entry->fd = new_fd;
		new_client_entry->open = true;
		pr_debug("adding receiver connection (fd %d)\n", new_client_entry->fd);
		pthread_mutex_lock(&client_lock);
		TAILQ_INSERT_TAIL(&client_queue_head, new_client_entry, entries);
		pthread_mutex_unlock(&client_lock);

		new_client_entry = NULL;
	}

	return NULL;
}

void *bcast_work()
{
	struct msg_entry *current = NULL;

	while (1) {
		pthread_mutex_lock(&msg_lock);
		while (TAILQ_EMPTY(&msg_queue_head)) {
			pthread_cond_wait(&msg_cond, &msg_lock);
		}

		/* get new entry and remove from queue */
		current = TAILQ_FIRST(&msg_queue_head);
		TAILQ_REMOVE(&msg_queue_head, current, entries);
		pthread_mutex_unlock(&msg_lock);

		/* TODO grace period to allow for new receivers */

		/* broadcast message */
		broadcast(current->msg);

		/* free message data after broadcast */
		free_ctmp_msg(current->msg);
		free(current);
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	int res;
	pthread_t dst_server_thread, bcast_thread;

	/* parse command-line arguments */
	parse_args(argc, argv, &init_args);
	pr_debug("extended = %d\n", init_args.extended);

	/* initialise client and message queues */
	TAILQ_INIT(&client_queue_head);
	TAILQ_INIT(&msg_queue_head);

	/* create destination server thread */
	res = pthread_create(&dst_server_thread, NULL, &dst_server, NULL);
	if (res != 0) {
		perror("pthread_create");
		exit(res);
	}

	/* TODO create broadcast thread */
	res = pthread_create(&bcast_thread, NULL, &bcast_work, NULL);
	if (res != 0) {
		perror("pthread_create");
		exit(res);
	}

	/* run source server */
	src_server();

	return EXIT_SUCCESS;
}
