#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

#include <netinet/in.h>

#include "ctmp.h"

/* TODO modify to parse whole header instead of reading byte-by-byte
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

	/* read in padding byte */
	read(sock_fd, &magic, 1);

	/* read in length (16-bit unsigned network order) */
	/* TODO remove magic constant */
	if ((bytes_read = read(sock_fd, &length, 2)) > 0) {
		/* convert to host byte order */
		length = ntohs(length);
		printf("length: %u\n", length);
	}

	/* read in rest of message, allocating buffer with chosen length */
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
