/// @file

#include <stdlib.h>
#include <errno.h>

#include "ctmp.h"
#include "msg_queue.h"
#include "timestamp.h"
#include "log.h"

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
		p_error("malloc", errno);
		exit(errno);
	}

	/* init timestamp */
	get_clock_time(&(*entry)->timestamp);

	(*entry)->msg = msg;

	/* init sent status bitmask and associated lock */
	(*entry)->sent = 0;
	pthread_rwlock_init(&(*entry)->sent_lock, NULL);
}

/**
 * @brief Get the next message queue entry to process
 * @param head message queue head
 * @param lock message queue lock
 * @param cond message queue condition variable
 * @param current current entry to process
 * @param prev previous message queue entry
 * @param to_start whether to return to the start of the queue or go to the next
 * entry (after `prev`)
 * @return `current` (unchanged) if it is not NULL, the first entry of the queue
 * if `to_start = true`, the entry after `prev` otherwise
 * @details Wait for the desired message queue to become available (added to the
 * queue) if `current` is NULL
 */
struct msg_entry *get_msg_entry(struct msg_queue *head, pthread_mutex_t *lock,
		pthread_cond_t *cond, struct msg_entry *current,
		struct msg_entry *prev, bool to_start)
{
	struct msg_entry *new_entry = NULL;

	if (TAILQ_EMPTY(head) || !current) {
		pthread_mutex_lock(lock);
		/* wait on different conditions depending on if the queue is
		 * empty or we are just waiting for the next message */
		if (TAILQ_EMPTY(head) || !prev) {
			/* wait for first entry */
			while (TAILQ_EMPTY(head)) {
				pthread_cond_wait(cond, lock);
			}
			new_entry = TAILQ_FIRST(head);
		} else {
			/* wait for next entry */
			while (!TAILQ_NEXT(prev, entries)) {
				pthread_cond_wait(cond, lock);
			}

			/* set current based on to_start argument */
			if (to_start) {
				new_entry = TAILQ_FIRST(head);
			} else {
				new_entry = TAILQ_NEXT(prev, entries);
			}
		}
		pthread_mutex_unlock(lock);
	} else {
		/* leave unchanged */
		new_entry = current;
	}

	return new_entry;
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
 * @brief Set a given bit of the `sent` bitmask to a given value
 * @param entry message queue entry to updat
 * @param thread_index thread index (= bit position) to update
 * @param val value to set the bit to
 */
void set_sent(struct msg_entry *entry, int thread_index, bool val)
{
	/* change message status to sent */
	pthread_rwlock_wrlock(&entry->sent_lock);
	set_bit(&entry->sent, thread_index, val);
	pthread_rwlock_unlock(&entry->sent_lock);
}

/* NOTE determine whether a receiver can send a given message i.e. the message
 * was sent after the receiver connected */
/**
 * @brief Determine whether a receiver can send a given message
 * @param entry message entry to check
 * @param thread_index receiver thread index
 * @param recv_start receiver connection timestamp
 * @details A receiver can send a message iff it was sent after the receiver
 * connected
 */
bool can_forward(struct msg_entry *entry, int thread_index,
		struct timespec recv_start)
{
	return (!is_sent(entry, thread_index) &&
			compare_times(&recv_start, &entry->timestamp));
}
