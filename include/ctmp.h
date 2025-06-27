/// @file

#include <stdio.h>
#include <stdint.h>

#define MAGIC 0xcc
#define HEADER_LENGTH 8  ///< CTMP header length

/**
 * @brief CTMP message
 */
struct ctmp_msg
{
	unsigned char header[HEADER_LENGTH];  ///< message header
	uint16_t len;  ///< length of data following header
	unsigned char *data;  ///< data following header
};

void free_ctmp_msg(struct ctmp_msg *msg);
struct ctmp_msg *parse_ctmp_msg(int sender_fd);
ssize_t send_ctmp_msg(int receiver_fd, struct ctmp_msg *msg);
