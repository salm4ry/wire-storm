/// @file

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "ctmp.h"
#include "msg_queue.h"

/* TODO naming */

/**
 * @brief Initialise a message queue entry
 * @param entry entry to initialise
 * @param msg CTMP message structure the entry should represent
 * @param num_threads number of worker threads
 * @details `num_threads` is used when initialising the `sent` member
 */
void init_msg_entry(struct msg_entry **entry, struct ctmp_msg *msg,
		int num_threads)
{
	(*entry) = malloc(sizeof(struct msg_entry));
	if (!(*entry)) {
		perror("malloc");
		exit(errno);
	}

	(*entry)->msg = msg;

	/* TODO init bitmask */
	(*entry)->sent = malloc(num_threads * sizeof(unsigned char));
	if (!(*entry)->sent) {
		perror("malloc");
		exit(errno);
	}
}

/**
 * @brief Free a message queue entry
 * @param entry entry to free
 */
void free_msg_entry(struct msg_entry *entry)
{
	free_ctmp_msg(entry->msg);
	/* TODO free bitmask */
	free(entry->sent);
	free(entry);
}
