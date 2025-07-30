/// @file

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "ctmp.h"
#include "msg_queue.h"
#include "timestamp.h"

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

	/* init sent status bitmask and associated lock */
	(*entry)->sent = 0;
	pthread_rwlock_init(&(*entry)->sent_lock, NULL);

	/*
	(*entry)->sent = malloc(num_threads * sizeof(bool));
	if (!(*entry)->sent) {
		perror("malloc");
		exit(errno);
	}
	*/
}

/**
 * @brief Free a message queue entry
 * @param entry entry to free
 */
void free_msg_entry(struct msg_entry *entry)
{
	free_ctmp_msg(entry->msg);
	/* free(entry->sent); */
	free(entry);
}

/**
 * @brief Determine if a given thread has already sent a given message
 * @param entry message queue entry to check
 * @param thread_index thread index (= bit position) to check message sent status of
 * @details The bit at the position `thread_index` in `entry->sent` represents
 * whether that thread has sent this message
 */
bool is_sent(struct msg_entry *entry, int thread_index)
{
	bool res;

	pthread_rwlock_rdlock(&entry->sent_lock);
	res = is_set(&entry->sent, thread_index);
	pthread_rwlock_unlock(&entry->sent_lock);

	return res;
}

/**
 * @brief Update the sent status bitmask at a given bit position
 * @param entry message queue entry to updat
 * @param thread_index thread index (= bit position) to update
 * @param val value to set the bit to
 */
void update_sent(struct msg_entry *entry, int thread_index, bool val)
{
	/* change message status to sent */
	pthread_rwlock_wrlock(&entry->sent_lock);
	update_bit(&entry->sent, thread_index, val);
	pthread_rwlock_unlock(&entry->sent_lock);
}

/* TODO
 * - fix for clock_gettime()
 * - docstring
 */
bool within_grace_period(struct msg_entry *entry, int grace_period)
{
	/* determine if a given message entry was sent within its grace period */
	// return (entry->timestamp + (grace_period * MS_PER_SEC) <= current_ts);
	return true;
}

/* NOTE determine whether a receiver can send a given message i.e. the message
 * was sent after the receiver connected */
/**
 * @brief Determine whether a receiver can send a given message
 * @param entry message entry to check
 * @param recv_start receiver connection timestamp
 * @param grace_period message grace period in seconds (TODO remove/fix)
 * @details A receiver can send a message iff it was sent after the receiver
 * connected
 */
bool can_forward(struct msg_entry *entry, struct timespec recv_start,
		int grace_period)
{
	struct timespec grace_period_ts;

	grace_period_ts = entry->timestamp;
	/* TODO use the grace period
	grace_period_ts.tv_nsec += 400 * MS_PER_NSEC;
	*/

	return (compare_times(&recv_start, &entry->timestamp) ||
		compare_times(&recv_start, &grace_period_ts));
}
