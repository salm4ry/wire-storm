/// @file

#include <stdio.h>
#include <stdint.h>

#define MAGIC 0xcc
#define HEADER_LENGTH 8  ///< CTMP header length

#define OPT_NORM 0x00
#define OPT_SEN 0x40

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

/* Wire Storm Reloaded */
uint16_t calc_checksum(struct ctmp_msg *msg);
struct ctmp_msg *parse_ctmp_msg_extended(int sender_fd);
