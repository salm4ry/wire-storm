/// @file

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "thread.h"
#include "timestamp.h"

/* TODO update docstrings */

/**
 * @brief Initialise worker threads
 * @param workers pointer to array of worker structs
 * @param num_workers number of workers stored in `workers`
 * @details Allocate memory for `workers`, then initialise the state values for
 * each worker
 */
void init_workers(struct worker_list *list, int num_workers)
{
	/* allocate thread array */
	list->num_workers = num_workers;
	list->workers = malloc(list->num_workers * sizeof(struct worker));
	if (!list->workers) {
		perror("malloc");
		exit(errno);
	}

	/* initialise status mask: 0 = available, 1 = busy
	 * NOTE: this does not include the third "ready" state since that's
	 * stored in the separate per-worker status field */
	list->mask.data = 0;
	pthread_mutex_init(&list->mask.lock, NULL);

	/* initialise thread states */
	for (int i = 0; i < list->num_workers; i++) {
		/* each worker thread is aware of their thread index */
		list->workers[i].args.thread_index = i;

		/* timestamp so that older messages are not sent */
		get_clock_time(&(list->workers)[i].args.timestamp);
		list->workers[i].status = THREAD_AVAILABLE;
		list->workers[i].args.status = &(list->workers[i]).status;
		list->workers[i].args.global_status = &list->mask;
	}
}

/**
 * @brief Find a non-busy thread
 * @param workers pointer to array of worker structs
 * @param num_workers number of workers stored in `workers`
 * @details A "non-busy" thread is either "available" (space exists but not yet
 created) or "ready" (thread created and waiting).
 * @return Thread index of non-busy worker thread on success, -1 on error
 */
int find_thread(struct worker_list *list)
{
	bool is_available;

	/* iterate through bits in the status bitmask */
	for (int i = 0; i < list->num_workers; i++) {
		pthread_mutex_lock(&list->mask.lock);
		is_available = !(is_set(&list->mask.data, i));
		pthread_mutex_unlock(&list->mask.lock);

		if (is_available) {
			return i;
		}
	}

	/* no available thread */
	return -1;
}
