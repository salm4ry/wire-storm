/// @file

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <pthread.h>

#include "socket.h"

/*
 * Socket code based on https://www.geeksforgeeks.org/c/socket-programming-cc/
 */

/**
 * @brief Set up socket server address
 * @param port server port
 * @return `sockaddr_in` describing the socket server address
 */
struct sockaddr_in server_address(int port)
{
	struct sockaddr_in address;

	address.sin_family = AF_INET; /* IPv4 */
	address.sin_addr.s_addr = INADDR_ANY; /* bind socket to all local interfaces */
	address.sin_port = htons(port);

	return address;
}

/**
 * @brief Create socket server listening on a given port
 * @param port TCP port to listen on
 * @return pointer to `struct server_socket` (including file descriptor) on
 * success, NULL on error
 */
struct server_socket *server_create(int port)
{
	int opt = 1;
	struct server_socket *server;

	server = malloc(sizeof (struct server_socket));
	if (!server) {
		perror("malloc");
		exit(errno);
	}

	/* set up socket */
	if ((server->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		goto cleanup;
	}

	/* set socket options */
	if (setsockopt(server->fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
				&opt, sizeof(opt))) {
		perror("setsockopt");
		goto cleanup;
	}

	/* bind to port */
	server->addr = server_address(port);

	if (bind(server->fd, (struct sockaddr *) &server->addr, sizeof(server->addr)) < 0) {
		perror("bind");
		goto cleanup;
	}

	/* listen for connections
	 * backlog = max number of pending connections */
	if (listen(server->fd, BACKLOG) < 0) {
		perror("listen");
		goto cleanup;
	}

	/* return new socket file descriptor and address */
	return server;

cleanup:
	free(server);
	return NULL;
}

/**
 * @brief Accept connection to a given socket server
 * @param server_fd server file descriptor
 * @param address server address
 * @return new socket file descriptor on success, negative value on error
 * (errno returned by the call to `accept()`)
 */
int server_accept(int server_fd, struct sockaddr_in address)
{
	int new_socket;
	socklen_t addrlen = sizeof(address);

	if ((new_socket = accept(server_fd, (struct sockaddr *) &address, &addrlen)) == -1) {
		perror("accept");
		/* check if we should retry */
		return -errno;
	}

	return new_socket;
}

/**
 * @brief Close socket server and related objects
 * @param server pointer struct containing server file descriptor and address
 * (to be freed)
 */
void server_close(struct server_socket *server)
{
	close(server->fd);
	free(server);
}
