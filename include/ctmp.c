#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

#include <netinet/in.h>

#include "ctmp.h"
#include "log.h"

/*
 * Validate the magic and length of the data before forwarding. Any messages
 * with an excessive length must be dropped
 * -> TODO define "excessive length"
 */

int parse_message(int sock_fd)
{
	int bytes_read = 0;
	uint16_t length;
	unsigned char header[HEADER_LENGTH], *message = NULL;

	/* read in header */
	bytes_read = read(sock_fd, &header, HEADER_LENGTH);
	if (bytes_read < 0) {
		/* read failed */
		perror("read");
		return -errno;
	} else if (bytes_read == 0) {
		/* no data sent */
		goto out;
	}

	/* validate magic byte (first byte of header) */
	if (header[0] != MAGIC) {
		pr_info("magic byte check failed: found 0x%02x\n", header[0]);
		return -1;
	}

	/* length = header bytes 2 and 3 */
	length = (header[3] << 8) + header[2];
	/* convert to host byte order */
	length = ntohs(length);
	pr_info("length: %u\n", length);

	/* read in rest of message, allocating buffer with chosen length
	 * +1 for NULL terminator */
	message = malloc((length+1) * sizeof(unsigned char));
	if (!message) {
		perror("malloc");
		exit(errno);
	}

	/* explicitly set last byte to NULL terminator before reading in message */
	message[length] = '\0';
	bytes_read = read(sock_fd, message, length);

	/* NOTE use this check to determine whether messages has the correct
	 * length value set in their header */
	if (bytes_read == length) {
		pr_debug("%s\n", message);
	}
	free(message);

out:
	return bytes_read;
}
