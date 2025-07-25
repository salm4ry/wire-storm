/// @file

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "ctmp.h"
#include "msg_queue.h"
#include "timestamp.h"

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

	/* init timestamp */
	get_clock_time(&(*entry)->timestamp);

	(*entry)->msg = msg;

	(*entry)->sent = malloc(num_threads * sizeof(bool));
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
	free(entry->sent);
	free(entry);
}

/* TODO docstrings for within_grace_period() and can_forward() */

/* TODO fix for clock_gettime()
 * determine if a given message entry was sent within its grace period */
bool within_grace_period(struct msg_entry *entry, int grace_period)
{
	// return (entry->timestamp + (grace_period * MS_PER_SEC) <= current_ts);
	return true;
}

/* NOTE determine whether a receiver can send a given message i.e. the message
 * was sent after the receiver connected */
bool can_forward(struct msg_entry *entry, struct timespec recv_start,
		int grace_period)
{

	/* TODO seconds are not precise enough: use gettimeofday() instead */
	return (compare_times(&recv_start, &entry->timestamp) &&
			within_grace_period(entry, grace_period));
}
