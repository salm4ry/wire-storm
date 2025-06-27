/// @file

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include <errno.h>
#include <pthread.h>
#include <sys/queue.h>

#include "include/log.h"
#include "include/socket.h"
#include "include/ctmp.h"

struct client_entry {
	int fd;  ///< socket file descriptor
	bool open;  ///< is the connection open?
	TAILQ_ENTRY(client_entry) entries;  ///< prev + next pointers for client queue
};

TAILQ_HEAD(client_list, client_entry);  ///< define client_list as a doubly linked tail queue
pthread_mutex_t client_lock = PTHREAD_MUTEX_INITIALIZER;
struct client_list client_queue_head;


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
			close(current->fd);
			TAILQ_REMOVE(&client_queue_head, current, entries);
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

	src_server = server_create(SRC_PORT);
	if (!src_server) {
		pr_err("error setting up server on port %d\n", SRC_PORT);
		exit(EXIT_FAILURE);
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
			current_msg = parse_ctmp_msg(src_socket);
			if (current_msg) {
				broadcast(current_msg);
				free_ctmp_msg(current_msg);
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
	struct client_entry *new_entry = NULL;

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

		new_entry = malloc(sizeof(struct client_entry));
		if (!new_entry) {
			perror("malloc");
			exit(errno);
		}

		new_entry->fd = new_fd;
		new_entry->open = true;
		pthread_mutex_lock(&client_lock);
		TAILQ_INSERT_TAIL(&client_queue_head, new_entry, entries);
		pthread_mutex_unlock(&client_lock);

		new_entry = NULL;
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	int res;
	pthread_t dst_server_thread;

	/* initialise client queue */
	TAILQ_INIT(&client_queue_head);

	/* create destination server thread */
	res = pthread_create(&dst_server_thread, NULL, &dst_server, NULL);
	if (res != 0) {
		perror("pthread_create");
		exit(res);
	}

	/* run source server */
	src_server();

	return EXIT_SUCCESS;
}
