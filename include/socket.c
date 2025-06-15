#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "socket.h"

/*
 * Socket code based on https://www.geeksforgeeks.org/c/socket-programming-cc/
 */

int server_create(int port)
{
	int server_fd, opt = 1;
	struct sockaddr_in address;

	/* set up socket */
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return -errno;
	}

	/* set socket options */
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
				&opt, sizeof(opt))) {
		perror("setsockopt");
		return -errno;
	}

	/* bind to port */
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
		perror("bind");
		return -errno;
	}

	/* listen for connections
	 * backlog = max number of pending connections */
	if (listen(server_fd, BACKLOG) < 0) {
		perror("listen");
		return -errno;
	}

	/* return new socket file descriptor */
	return server_fd;
}

int server_accept(int server_fd, struct sockaddr_in address)
{
	int new_socket;
	socklen_t addrlen = sizeof(address);

	if ((new_socket = accept(server_fd, (struct sockaddr *) &address, &addrlen)) < 0) {
		perror("accept");
		return -errno;
	}

	return new_socket;
}
