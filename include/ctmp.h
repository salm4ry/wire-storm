/// @file

#include <stdint.h>

#define MAGIC 0xcc
#define HEADER_LENGTH 8

/* TOOD docstring */
struct ctmp_msg
{
	uint16_t len;
	char *data;
};

struct ctmp_msg *parse_msg(int sender_fd);
void free_msg(struct ctmp_msg *msg);
