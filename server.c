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
	int fd;
	bool open;
	TAILQ_ENTRY(client_entry) entries;
};

TAILQ_HEAD(client_list, client_entry);
pthread_mutex_t client_lock = PTHREAD_MUTEX_INITIALIZER;
struct client_list client_list_head;

void *dst_server(void *data)
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
			/* TODO exit? */
		}

		new_entry = malloc(sizeof(struct client_entry));
		if (!new_entry) {
			perror("malloc");
			exit(errno);
		}

		new_entry->fd = new_fd;
		new_entry->open = true;
		// pthread_mutex_lock(&client_lock);
		TAILQ_INSERT_TAIL(&client_list_head, new_entry, entries);
		// pthread_mutex_unlock(&client_lock);

		new_entry = NULL;
	}


	return NULL;
}

void broadcast(struct ctmp_msg *msg)
{
	ssize_t bytes_sent;
	struct client_entry *current = NULL, *next = NULL;

	pthread_mutex_lock(&client_lock);
	current = TAILQ_FIRST(&client_list_head);
	pthread_mutex_unlock(&client_lock);

	while (current) {
		if (current->open) {
			bytes_sent = send(current->fd, msg->header, HEADER_LENGTH, MSG_NOSIGNAL);
			bytes_sent = send(current->fd, msg->data, msg->len, MSG_NOSIGNAL);
		}

		if (bytes_sent < 0) {
			current->open = false;
		}

		pthread_mutex_lock(&client_lock);
		next = TAILQ_NEXT(current, entries);
		pthread_mutex_unlock(&client_lock);

		if (!current->open) {
			close(current->fd);
			TAILQ_REMOVE(&client_list_head, current, entries);
			free(current);
		}

		current = next;
	}
}

int main(int argc, char *argv[])
{
	int src_socket, res;
	struct server_socket *src_server = NULL;
	pthread_t dst_server_thread;

	struct ctmp_msg *current_msg = NULL;

	src_server = server_create(SRC_PORT);
	if (!src_server) {
		pr_err("error setting up server on port %d\n", SRC_PORT);
		return EXIT_FAILURE;
	}

	TAILQ_INIT(&client_list_head);

	res = pthread_create(&dst_server_thread, NULL, &dst_server, NULL);
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
			/* TODO add to linked list */
			if (current_msg) {
				broadcast(current_msg);
			}
		} while (current_msg);

		pr_debug("closing src connection...\n");
		close(src_socket);
	}

	return EXIT_SUCCESS;
}
