/// @file

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "include/socket.h"
#include "include/ctmp.h"
#include "include/log.h"

struct dst_client_args {
	int fd;
	pthread_mutex_t *lock;
	pthread_cond_t *cond;
};

struct dst_server_args {
	pthread_mutex_t *lock;
	pthread_cond_t *cond;
};

/*
 * Allow a single source client to connect on port 33333
 *
 * Allow multiple destination clients to connect on port 44444
 *
 * Accept CTMP messages from the source and forward to all destination clients.
 * These should be forwarded in the order they are received
 */

struct server_socket *src_server;

/* TODO better explanation
 * condition for message existing to relay */
pthread_mutex_t msg_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t msg_cond = PTHREAD_COND_INITIALIZER;
struct ctmp_msg *current_msg = NULL;

void *handle_conn(void *data)
{
	ssize_t bytes_sent = 0;
	struct dst_client_args *args = (struct dst_client_args *) data;

	pr_debug("waiting for messages...\n");
	pthread_mutex_lock(args->lock);

	do {
		while (!current_msg) {
			pthread_cond_wait(args->cond, args->lock);
		}

		/* send message */
		pr_debug("sending message\n");
		bytes_sent = send(args->fd, current_msg->data, current_msg->len, 0);
	} while (bytes_sent > 0);

	close(args->fd);
	free(args);
	return NULL;
}

void *dst_server(void *data)
{
	int res;
	struct server_socket *dst_server;
	struct dst_server_args *server_args = (struct dst_server_args *) data;

	dst_server = server_create(DST_PORT);
	if (!dst_server) {
		pr_err("error setting up server on port %d\n", DST_PORT);
		exit(EXIT_FAILURE);
	}

	while (1) {
		struct dst_client_args *client_args = NULL;
		pthread_t current_thread;

		client_args = malloc(sizeof(struct dst_client_args));
		if (!client_args) {
			perror("malloc");
			exit(errno);
		}

		/* wait for connections */
		client_args->lock = server_args->lock;
		client_args->cond = server_args->cond;

		client_args->fd = server_accept(dst_server->fd, dst_server->addr);
		if (client_args->fd < 0) {
			pr_err("error accepting connection to port %d\n", DST_PORT);
		}

		/* create thread for new client */
		res = pthread_create(&current_thread, NULL, handle_conn, client_args);
		if (res != 0) {
			perror("pthread_create");
			/* TODO how to handle */
		}

		/* TODO decide how to store threads */
		/* TODO store messages in linked list/queue? */
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	int src_socket, res;
	pthread_t dst_server_thread;
	struct dst_server_args *dst_args = NULL;

	/* set up servers */
	src_server = server_create(SRC_PORT);
	if (!src_server) {
		pr_err("error setting up server on port %d\n", SRC_PORT);
		return EXIT_FAILURE;
	}

	dst_args = malloc(sizeof(struct dst_server_args));
	if (!dst_args) {
		perror("malloc");
		exit(errno);
	}

	dst_args->lock = &msg_lock;
	dst_args->cond = &msg_cond;
	res = pthread_create(&dst_server_thread, NULL, &dst_server, dst_args);
	if (res != 0) {
		perror("pthread_create");
		exit(res);
	}

	while (1) {
		src_socket = server_accept(src_server->fd, src_server->addr);
		if (src_socket < 0) {
			pr_err("error accepting connection to port %d\n", SRC_PORT);
			exit(-src_socket);
		}

		do {
			free_msg(current_msg);
			current_msg = parse_msg(src_socket);
			pthread_cond_broadcast(&msg_cond);
		} while (current_msg);

		pr_debug("closing connection...\n");
		/* close connected socket */
		close(src_socket);
	}

	return EXIT_SUCCESS;
}
