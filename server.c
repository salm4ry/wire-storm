#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>

#include <sys/param.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

/* TODO move socket code into separate functions/header
 *
 * Socket code based on https://www.geeksforgeeks.org/c/socket-programming-cc/
 */
int main(int argc, char *argv[])
{
	int server_fd, new_socket, opt = 1;
	struct sockaddr_in address;
	socklen_t addrlen = sizeof(address);

	/* set up socket
	 * TODO move to own function? */
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(errno);
	}

	/* attach to port 33333 */
	if (setsockopt(server_fd, SOL_SOCKET,
				SO_REUSEADDR | SO_REUSEPORT, &opt,
				sizeof(opt))) {
		perror("setsockopt");
		exit(errno);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(SERVER_PORT);

	if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
		perror("bind");
		exit(errno);
	}

	if (listen(server_fd, BACKLOG) < 0) {
		perror("listen");
		exit(errno);
	}

	/* TODO handle client closing and accept a new connection */
	if ((new_socket = accept(server_fd, (struct sockaddr *) &address, &addrlen)) < 0) {
		perror("accept");
		exit(errno);
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
