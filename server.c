#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "include/socket.h"
#include "include/ctmp.h"

#define SERVER_PORT 33333
#define BACKLOG 3 /* max number of pending connections */
#define MAGIC 0xcc /* magic byte */

/*
 * Allow a single source client to connect on port 33333
 *
 * Allow multiple destination clients to connect on port 44444
 *
 * Accept CTMP messages from the source and forward to all destination clients.
 * These should be forwarded in the order they are received
 */

int main(int argc, char *argv[])
{
	int server_fd, new_socket;
	struct sockaddr_in address;

	server_fd = server_create(SERVER_PORT);
	if (server_fd < 0) {
		fprintf(stderr, "error setting up server on port %d\n", SERVER_PORT);
		exit(-server_fd);
	}

	/* TODO handle client closing and accept a new connection */

	/* TODO clean up */
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(SERVER_PORT);
	new_socket = server_accept(server_fd, address);
	if (new_socket < 0) {
		fprintf(stderr, "error accepting connection to port %d\n", SERVER_PORT);
		exit(-new_socket);
	}

	while (1) {
		parse_message(new_socket);
	}

	/* close connected socket */
	close(new_socket);

	/* close listening socket */
	close(server_fd);

	return EXIT_SUCCESS;
}
