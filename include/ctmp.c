/// @file

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

/**
 * @brief Parse CTMP message on a given socket
 * @param sender_fd file desciptor to read message from
 * @param receiver_fd file descriptor to forward message to (TODO handle
 * multiple receivers using multithreaded abstraction)
 * @return number of bytes read on success, -1 if the magic byte check failed,
 * negative error code otherwise
 */
int parse_message(int sender_fd, int receiver_fd)
{
	int bytes_read = 0;
	uint16_t length;
	unsigned char header[HEADER_LENGTH], *message = NULL;

	/* read in header */
	bytes_read = read(sender_fd, &header, HEADER_LENGTH);
	if (bytes_read < 0) {
		/* read failed */
		perror("read");
		bytes_read = -errno;
		goto out;
	} else if (bytes_read == 0) {
		/* no data sent */
		goto out;
	}

	/* validate magic byte (first byte of header) */
	if (header[0] != MAGIC) {
		pr_info("magic byte check failed: found 0x%02x\n", header[0]);
		bytes_read = -1;
		goto out;
	}

	/* length = header bytes 2 and 3 */
	length = (header[3] << 8) + header[2];
	/* convert to host byte order */
	length = ntohs(length);

	/* read in rest of message, allocating buffer with chosen length
	 * +1 for NULL terminator */
	message = malloc((length+1) * sizeof(unsigned char));
	if (!message) {
		perror("malloc");
		exit(errno);
	}

	/* explicitly set last byte to NULL terminator before reading in message */
	message[length] = '\0';
	bytes_read = read(sender_fd, message, length);

	/* determine whether the message has the correct length value set */
	pr_info("length from header: %u, bytes read: %u\n", length, bytes_read);
	if (bytes_read == length) {
		pr_debug("%s\n", message);

		/* TODO replace with send to multiple receivers */
		send(receiver_fd, message, length, 0);
	} else {
		/* length does not match */
		pr_debug("bytes read %u does not match length %u\n",
				bytes_read, length);
		bytes_read = -1;
	}
	free(message);

out:
	return bytes_read;
}
