/// @file

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "thread.h"
#include "timestamp.h"

/**
 * @brief Initialise worker threads
 * @param workers pointer to array of worker structs
 * @param num_workers number of workers stored in `workers`
 * @details Allocate memory for `workers`, then initialise the state values for
 * each worker
 */
void init_workers(struct worker **workers, int num_workers)
{
	/* allocate thread array */
	*workers = malloc(num_workers * sizeof(struct worker));
	if (!(*workers)) {
		perror("malloc");
		exit(errno);
	}

	/* initialise thread states */
	for (int i = 0; i < num_workers; i++) {
		/* each worker thread is aware of their thread index */
		(*workers)[i].args.thread_index = i;

		/* NOTE timestamp so that older messages are not sent */
		get_clock_time(&(*workers)[i].args.timestamp);
		(*workers)[i].status = THREAD_AVAILABLE;
		(*workers)[i].args.status = &(*workers[i]).status;
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
int find_thread(struct worker **workers, int num_workers)
{
	for (int i = 0; i < num_workers; i++) {
		if ((*workers)[i].status != THREAD_BUSY) {
			return i;
		}
	}

	/* no available thread */
	return -1;
}
