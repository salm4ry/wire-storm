/// @file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
	/* TODO check correct padding */

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

/**
 * @brief Calculate CTMP checksum
 * @details Checksum is the 16-bit one's complement sum of all 16-bit words in
 * the header and the data. For purposes of computing the checksum, the value of
 * the checksum field is filled with 0xCC bytes
 * @param msg CTMP message to calculate checksum for
 * @return calculated checksum
 */
uint16_t calc_checksum(struct ctmp_msg *msg)
{
	register long sum = 0;
	uint16_t count;
	uint16_t *addr;
	unsigned char header[HEADER_LENGTH];

	/* copy header to preserve original checksum value */
	memcpy(header, msg->header, HEADER_LENGTH);

	/* based on RFC 1071 implementation of the IP checksum:
	 * https://datatracker.ietf.org/doc/html/rfc1071
	 */

	/* fill checksum field (bytes 4 and 5) with 0xCC */
	header[4] = 0xCC;
	header[5] = 0xCC;

	addr = (uint16_t *) header;
	/* header portion */
	for (int i = 0; i < HEADER_LENGTH; i += 2) {
		sum += (uint16_t) *addr++;
	}

	addr = (uint16_t *) msg->data;
	count = msg->len;
	/* data portion */
	while (count > 1) {
		sum += (uint16_t) *addr++;
		count -= 2;
	}

	/* add leftover byte (if any) */
	if (count > 0) {
		sum += * (uint8_t *) addr;
	}

	/* fold to 16 bits (TODO better explanation) */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	/* one's complement of the one's complement sum */
	return ~sum;
}

/**
 * @brief Parse an extended CTMP message
 * @details Calculate and validate the checksum for messages where the sensitive
 * options bit is 1, dropping messages with invalid checksums
 * @param sender_fd file desciptor to read message from
 * @return parsed message structure, NULL on error (invalid message/failed
 * to read)
 */
struct ctmp_msg *parse_ctmp_msg_extended(int sender_fd)
{
	/* TODO for messages where the sensitive options bit is 1, the checksum
	 * must be calculated and validated
	 *
	 * Any message with an invalid checksum must be dropped with an error
	 * logged
	 */
	int bytes_read = 0;
	struct ctmp_msg *msg = NULL;
	uint16_t header_checksum, calculated_checksum;

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
		goto out;
	}

	/* check options (byte 1) */
	if ((msg->header[1] ^ OPT_SEN) == 0) {
		/* validate checksum */
		header_checksum = (msg->header[5] << 8) + msg->header[4];
		calculated_checksum = calc_checksum(msg);
		pr_debug("checksum in header: %u, calculated: %u\n", header_checksum, calculated_checksum);

		if (header_checksum != calculated_checksum) {
			pr_err("invalid message: checksum validation failed (found %u, expected %u)\n");
			free_ctmp_msg(msg);
			msg = NULL;
		}
	} else if ((msg->header[1] ^ OPT_NORM) != 0) {
		pr_err("invalid options (0%02x)\n", msg->header[1]);
	}

out:
	return msg;
}
