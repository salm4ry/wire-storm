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
 * @brief Free a given `struct ctmp_msg` object
 * @param msg object to free
 */
void free_ctmp_msg(struct ctmp_msg *msg)
{
	if (msg) {
		if (msg->data) {
			free(msg->data);
		}
		free(msg);
	}
}

/**
 * @brief Parse CTMP message on a given socket
 * @param sender_fd file desciptor to read message from
 * @return parsed message structure, NULL on error (invalid message/failed
 * to read)
 */
struct ctmp_msg *parse_ctmp_msg(int sender_fd)
{
	int bytes_read = 0;
	struct ctmp_msg *msg = NULL;

	msg = malloc(sizeof(struct ctmp_msg));
	if (!msg) {
		perror("malloc");
		exit(errno);
	}

	/* read in header */
	bytes_read = read(sender_fd, msg->header, HEADER_LENGTH);
	if (bytes_read < 0) {
		/* read failed */
		perror("read");
		goto out;
	} else if (bytes_read == 0) {
		/* no data sent */
		free(msg);
		msg = NULL;
		goto out;
	}

	/* validate magic byte (first byte of header) */
	if (msg->header[0] != MAGIC) {
		pr_err("invalid message: magic byte check failed (found 0x%02x)\n", msg->header[0]);
		free(msg);
		msg = NULL;
		goto out;
	}

	msg->data = NULL;

	/* length = header bytes 2 and 3 */
	msg->len = (msg->header[3] << 8) + msg->header[2];
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
	pr_debug("length from header: %u, bytes read: %u\n", msg->len, bytes_read);
	if (bytes_read != msg->len) {
		/* length does not match */
		pr_err("invalid message: bytes read %u does not match length %u\n",
				bytes_read, msg->len);
		free_ctmp_msg(msg);
		msg = NULL;
	}

out:
	return msg;
}


/**
 * @brief Send a CTMP message
 * @param receiver_fd file descriptor of receiver
 * @param msg message to send
 * @return number of bytes sent (negative on error)
 */
ssize_t send_ctmp_msg(int receiver_fd, struct ctmp_msg *msg)
{
	ssize_t bytes_sent = 0;

	/* send header */
	bytes_sent = send(receiver_fd, msg->header, HEADER_LENGTH, MSG_NOSIGNAL);
	if (bytes_sent < 0) {
		goto out;
	}

	bytes_sent = send(receiver_fd, msg->data, msg->len, MSG_NOSIGNAL);

out:
	return bytes_sent;
}
