/// @file

#include <stdio.h>
#include <stdint.h>

#define MAGIC 0xcc
#define HEADER_LENGTH 8  ///< CTMP header length
#define PADDING 0x00
#define NUM_PADDING_BYTES 5  ///< number of padding bytes in base CTMP

#define LENGTH_POS 2 ///< first byte of message length: 2 bytes long
#define OPTIONS_POS 1  ///< options = first byte of header in extended version
#define CHECKSUM_POS 4  ///< first byte of checksum: 2 bytes long

#define PADDING_START 4  ///< start of base CTMP padding (excluding byte 1)
#define PADDING_END 7  ///< end of base CTMP padding (excluding byte 1)

#define EXT_PADDING_START 2  ///< start of extended CTMP padding
#define EXT_PADDING_END 3  ///< end of extended CTMP padding

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
