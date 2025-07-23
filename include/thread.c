#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "thread.h"

/* TODO docstrings */

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
		(*workers)[i].status = THREAD_AVAILABLE;
		(*workers)[i].args.status = &(*workers[i]).status;
	}
}

int find_thread(struct worker **workers, int num_workers)
{
	for (int i = 0; i < num_workers; i++) {
		/* FIXME */
		if ((*workers)[i].status == THREAD_READY
				|| (*workers)[i].status == THREAD_AVAILABLE) {
			return i;
		}
	}

	/* no available thread */
	return -1;
}
