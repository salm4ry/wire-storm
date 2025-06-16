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
	int new_socket;
	struct server_socket *sender_server;

	sender_server = server_create(SERVER_PORT);
	if (!sender_server) {
		fprintf(stderr, "error setting up server on port %d\n", SERVER_PORT);
		return EXIT_FAILURE;
	}

	/* TODO handle client closing and accept a new connection */

	new_socket = server_accept(sender_server->fd, sender_server->addr);
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
	close(sender_server->fd);
	free(sender_server);

	return EXIT_SUCCESS;
}
