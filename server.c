#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "./include/socket.h"

#define SERVER_PORT 33333
#define BACKLOG 3 /* max number of pending connections */
#define MAGIC 0xcc /* magic byte */

/*
 * Allow a single source client to connect on port 33333
 *
 * Allow multiple destination clients to connect on port 44444
 *
 * Accept CTMP messages from the soruce and forward to all destination clients.
 * These should be forwarded in the order they are received
 *
 * Validate the magic and length of the data before forwarding. Any messages
 * with an excessive length must be dropped
 * -> TODO define "excessive length"
 */


int parse_message(int sock_fd)
{
	int bytes_read;
	uint16_t length;
	unsigned char magic;
	unsigned char *message = NULL;

	/* validate magic byte */
	if ((bytes_read = read(sock_fd, &magic, 1)) > 0) {
		if (magic != MAGIC) {
			printf("invalid data\n");
			return 1;
		} else {
			printf("magic byte found!\n");
		}
	}

	/* read in padding byte
	 * TODO find a better way to handle padding */
	read(sock_fd, &magic, 1);

	/* read in length (16-bit unsigned network order) */
	if ((bytes_read = read(sock_fd, &length, 2)) > 0) {
		/* convert to host byte order */
		length = ntohs(length);
		printf("length: %u\n", length);
	}

	/* read in rest of message */
	/* allocate buffer */
	/* TODO define "excessive length" and check here */
	message = malloc(length * sizeof(unsigned char));
	if (!message) {
		perror("malloc");
		exit(errno);
	}

	bytes_read = read(sock_fd, message, length);
	if (bytes_read > 0) {
		for (int i = 0; i < length; i++) {
			printf("\\x%02x", message[i]);
		}
		printf("\n");
	}
	free(message);

	return 0;
}

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
