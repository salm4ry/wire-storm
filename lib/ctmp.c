/// @file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

#include <netinet/in.h>

#include "ctmp.h"
#include "log.h"

/**
 * @brief Read a message of a given length from a given file descriptor
 * @param fd file descriptor to read from
 * @param buf buffer to store message in
 * @param len length of message to be read in bytes
 * @return 0 on success, negative error code otherwise
 *
 * @details Continue reading until there is nothing left to read or the expected
 * length has been read.
 *
 * Return values:
 * - 0: correct number of bytes (= `len`) read
 * - -1: mismatch between expected and actual number of bytes read
 * - other negative error code: `read()` failure
 */
int read_msg(int fd, unsigned char *buf, uint16_t len)
{
	ssize_t bytes_read = 0, total_bytes_read = 0;

	do {
		bytes_read = read(fd, &buf[total_bytes_read],
				len - total_bytes_read);
		if (bytes_read < 0) {
			/* check if we should retry the read */
			if (errno == EINTR) {
				/* interrupted by syscall: retry */
				continue;
			} else {
				perror("read");
				return -errno;
			}
		} else if (bytes_read == 0) {
			break;
		}
		total_bytes_read += bytes_read;
		pr_debug("read %d, total %d, expected %d\n",
				bytes_read, total_bytes_read, len);
	} while (total_bytes_read < len);

	if (total_bytes_read == 0) {
		pr_debug("nothing to read\n");
		return -1;
	}

	if (total_bytes_read == len) {
		return 0;
	} else {
		return -1;
	}
}

/**
 * @brief Send a message of a given length to a given file descriptor
 * @param fd file descriptor to send to
 * @param buf data to send
 * @param len length of data to send in bytes
 * @return 0 on success, negative error code otherwise
 *
 * @details Continue sending until the full message has been sent/send fails.
 *
 * Return values:
 * - 0: correct number of bytes = (`len`) sent
 * - -1: mismatch between expected and actual number of bytes sent
 * - other negative error code: `send()` failure
 */
int send_msg(int fd, unsigned char *buf, uint16_t len)
{
	ssize_t bytes_sent = 0, total_bytes_sent = 0;

	do {
		/* MSG_NOSIGNAL: don't generate SIGIPE if the socket has closed
		 * the connection (errno still set to EPIPE) */
		bytes_sent = send(fd, &buf[total_bytes_sent],
				len - total_bytes_sent, MSG_NOSIGNAL);
		if (bytes_sent < 0) {
			/* check if we should retry */
			if (errno == EINTR) {
				/* interrupted by syscall: retry */
				continue;
			} else {
				return -errno;
			}
		} else if (bytes_sent == 0) {
			break;
		}
		total_bytes_sent += bytes_sent;
		/*
		pr_debug("sent %d, total %d, expected %d\n",
				bytes_sent, total_bytes_sent, len);
		*/
	} while (total_bytes_sent < len);

	if (total_bytes_sent == len) {
		return 0;
	} else {
		return -1;
	}
}

/**
 * @brief Read in CTMP header
 * @param sender_fd file descriptor to read header from
 * @param msg `struct ctmp_msg` to store header in
 * @return number of bytes read (negative error code on failure)
 */
int read_ctmp_header(int sender_fd, struct ctmp_msg *msg)
{
	/* read in header */
	return read_msg(sender_fd, msg->header, HEADER_LENGTH);
}

/**
 * @brief Validate CTMP header magic byte
 * @param msg message to validate magic byte of
 * @return true if valid magic byte, false otherwise
 */
bool valid_magic(struct ctmp_msg *msg)
{
	return (msg->header[0] == MAGIC);
}

/**
 * @brief Validate CTMP header padding
 * @details Header bytes 1 and 4-7 should contain 0x00
 * @param msg message to validate padding of
 * @param extended whether extended mode is enabled
 * @return true if valid padding, false otherwise
 */
bool valid_padding(struct ctmp_msg *msg, bool extended)
{
	int start, end;

	/* set padding start and end depending on protocol version */
	if (extended) {
		start = EXT_PADDING_START;
		end = EXT_PADDING_END;
	} else {
		start = PADDING_START;
		end = PADDING_END;

		/* only check options byte if not in extended mode */
		if ((msg->header[OPTIONS_POS] & PADDING) != 0) {
			return false;
		}
	}

	for (int i = start; i <= end; i++) {
		if ((msg->header[i] & PADDING) != 0) {
			return false;
		}
	}

	if (!extended) {
		if ((msg->header[OPTIONS_POS] & PADDING) != 0) {
			return false;
		}
	}

	return true;
}

/**
 * @brief Set CTMP message length
 * @details Message length is stored in header bytes 2 and 3 as an unsigned
 * 16-bit network-order integer
 * @param msg `struct ctmp_msg` to set length of
 */
void set_msg_length(struct ctmp_msg *msg)
{
	/* length = header bytes 2 and 3 */
	msg->len = (msg->header[LENGTH_POS+1] << 8) + msg->header[LENGTH_POS];
	/* convert from network to host order */
	msg->len = ntohs(msg->len);
}

/**
 * @brief Read CTMP data based on header information
 * @param sender_fd file descriptor to read data from
 * @param msg `struct ctmp_msg` to store data in
 * @return number of bytes read (negative error code on failure)
 */
int read_ctmp_data(int sender_fd, struct ctmp_msg *msg)
{
	int bytes_read = 0;

	msg->data = malloc((msg->len+1) * sizeof(unsigned char));
	if (!msg->data) {
		perror("malloc");
		exit(errno);
	}

	/* explicitly set last byte to NULL terminator before reading in message */
	msg->data[msg->len] = '\0';

	bytes_read = read_msg(sender_fd, msg->data, msg->len);
	return bytes_read;
}

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
	int res = 0;
	struct ctmp_msg *msg = NULL;

	msg = malloc(sizeof(struct ctmp_msg));
	if (!msg) {
		perror("malloc");
		exit(errno);
	}
	msg->data = NULL;

	/* read in header */
	res = read_ctmp_header(sender_fd, msg);
	if (res < 0) {
		free_ctmp_msg(msg);
		msg = NULL;
		goto out;
	}

	/* validate magic byte (first byte of header) */
	if (!valid_magic(msg)) {
		pr_err("invalid message: magic byte check failed (found 0x%02x)\n",
				msg->header[0]);
		free_ctmp_msg(msg);
		msg = NULL;
		goto out;
	}

	/* check padding correctly set to 0x00s */
	if (!valid_padding(msg, false)) {
		pr_err("invalid message: incorrect padding\n");
	}

	/* get message length from header */
	set_msg_length(msg);

	res = read_ctmp_data(sender_fd, msg);
	if (res < 0) {
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
	int res = 0;

	/* send header */
	res = send_msg(receiver_fd, msg->header, HEADER_LENGTH);
	if (res < 0) {
		goto out;
	}

	res = send_msg(receiver_fd, msg->data, msg->len);

out:
	return res;
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
	header[CHECKSUM_POS] = 0xCC;
	header[CHECKSUM_POS+1] = 0xCC;

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

	/* fold to 16 bits by adding 16-bit segments */
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
	int res = 0;
	struct ctmp_msg *msg = NULL;
	uint16_t header_checksum, calculated_checksum;

	msg = malloc(sizeof(struct ctmp_msg));
	if (!msg) {
		perror("malloc");
		exit(errno);
	}
	msg->data = NULL;

	/* read in header */
	res = read_ctmp_header(sender_fd, msg);
	if (res < 0) {
		/* read failed */
		free_ctmp_msg(msg);
		msg = NULL;
		goto out;
	}

	/* validate magic byte (first byte of header) */
	if (!valid_magic(msg)) {
		pr_err("invalid message: magic byte check failed (found 0x%02x)\n", msg->header[0]);
		free(msg);
		msg = NULL;
		goto out;
	}

	/* check padding correctly set to 0x00s */
	if (!valid_padding(msg, true)) {
		pr_err("invalid message: incorrect padding\n");
	}

	set_msg_length(msg);

	res = read_ctmp_data(sender_fd, msg);
	if (res < 0) {
		free_ctmp_msg(msg);
		msg = NULL;
		goto out;
	}

	/* check options */
	switch (msg->header[OPTIONS_POS]) {
	case OPT_NORM:
		/* do nothing */
		break;
	case OPT_SEN:
		/* validate checksum */
		header_checksum = (msg->header[CHECKSUM_POS+1] << 8) + msg->header[CHECKSUM_POS];
		calculated_checksum = calc_checksum(msg);
		pr_debug("checksum in header: %u, calculated: %u\n", header_checksum, calculated_checksum);

		if (header_checksum != calculated_checksum) {
			pr_err("invalid message: checksum validation failed (found %u, expected %u)\n",
					calculated_checksum, header_checksum);
			free_ctmp_msg(msg);
			msg = NULL;
		}
		break;
	default:
		pr_err("invalid options (0x%02x)\n", msg->header[OPTIONS_POS]);
		break;
	}

out:
	return msg;
}
