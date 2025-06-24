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
struct ctmp_msg *parse_msg(int sender_fd)
{
	int bytes_read = 0;
	unsigned char header[HEADER_LENGTH];
	struct ctmp_msg *msg = NULL;

	/* read in header */
	bytes_read = read(sender_fd, &header, HEADER_LENGTH);
	if (bytes_read < 0) {
		/* read failed */
		perror("read");
		goto out;
	} else if (bytes_read == 0) {
		/* no data sent */
		goto out;
	}

	/* validate magic byte (first byte of header) */
	if (header[0] != MAGIC) {
		pr_info("magic byte check failed: found 0x%02x\n", header[0]);
		goto out;
	}

	msg = malloc(sizeof(struct ctmp_msg));
	if (!msg) {
		perror("malloc");
		exit(errno);
	}

	msg->data = NULL;

	/* length = header bytes 2 and 3 */
	msg->len = (header[3] << 8) + header[2];
	/* convert to host byte order */
	msg->len = ntohs(msg->len);

	/* read in rest of message, allocating buffer with chosen length
	 * +1 for NULL terminator */
	msg->data = malloc((msg->len+1) * sizeof(unsigned char));
	if (!msg->data) {
		perror("malloc");
		exit(errno);
	}

	/* explicitly set last byte to NULL terminator before reading in message */
	msg->data[msg->len] = '\0';
	bytes_read = read(sender_fd, msg->data, msg->len);

	/* determine whether the message has the correct length value set */
	pr_info("length from header: %u, bytes read: %u\n", msg->len, bytes_read);
	if (bytes_read == msg->len) {
		/* pr_debug("%s\n", msg->data); */

		/* TODO replace with send to multiple receivers */
		/* send(receiver_fd, msg->data, length, 0); */
	} else {
		/* length does not match */
		pr_debug("bytes read %u does not match length %u\n",
				bytes_read, msg->len);
	}

out:
	return msg;
}

void free_msg(struct ctmp_msg *msg)
{
	if (msg) {
		if (msg->data) {
			free(msg->data);
		}
		free(msg);
	}
}
