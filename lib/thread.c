/// @file

#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>

#include "thread.h"
#include "timestamp.h"
#include "log.h"

/**
 * @brief Initialise worker threads
 * @param list pointer to worker thread list struct
 * @param num_workers number of workers to set up
 * @details Allocate memory for `workers`, then initialise the state values
 * (global and per-worker) and timestamps
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
	list->threads_status.data = 0;
	pthread_mutex_init(&list->threads_status.lock, NULL);

	/* initialise thread states */
	for (int i = 0; i < list->num_workers; i++) {
		/* each worker thread is aware of their thread index */
		list->workers[i].args.thread_index = i;

		/* timestamp so that older messages are not sent */
		get_clock_time(&(list->workers)[i].args.timestamp);
		list->workers[i].status = THREAD_AVAILABLE;
		list->workers[i].args.self_status = &(list->workers[i]).status;
		list->workers[i].args.threads_status = &list->threads_status;
	}
}

/**
 * @brief Find an idle thread
 * @param list pointer to worker thread list struct
 * @details A "idle" thread is either "available" (space exists but not yet
 created) or "ready" (thread created and waiting).
 * @return Thread index of idle worker thread on success, -1 on error
 */
int find_idle_thread(struct worker_list *list)
{
	bool is_idle;

	/* iterate through bits in the status bitmask */
	for (int i = 0; i < list->num_workers; i++) {
		pthread_mutex_lock(&list->threads_status.lock);
		is_idle = !(is_set(&list->threads_status.data, i));
		pthread_mutex_unlock(&list->threads_status.lock);

		if (is_idle) {
			return i;
		}
	}

	/* all threads are busy */
	return -1;
}

/**
 * @brief Initialise worker thread arguments and status
 * @param list pointer to worker thread list struct
 * @param thread_index index of thread to use
 * @param client_fd client file descriptor to use
 * @param client_ts client timestamp to use
 * @details A worker thread is ready to be created after this function completes
 */
void init_thread_info(struct worker_list *list, int thread_index,
		int client_fd, struct timespec client_ts)
{
	struct worker *thread;
	thread = &(list->workers[thread_index]);

	/* initialise thread arguments */
	pthread_mutex_init(&thread->args.lock, NULL);
	pthread_cond_init(&thread->args.cond, NULL);
	thread->args.client_fd = client_fd;

	// thread->status = THREAD_BUSY;
	*(thread->args.self_status) = THREAD_BUSY;

	pthread_mutex_lock(&list->threads_status.lock);
	set_bit(&list->threads_status.data, thread_index, true);
	pthread_mutex_unlock(&list->threads_status.lock);

	thread->args.timestamp = client_ts;
}

/**
 * @brief Update a waiting worker thread's information then wake it up
 * @param list pointer to worker thread list struct
 * @param thread_index index of thread to update
 * @param client_fd new client file descriptor to set
 * @param client_ts new client timestamp to set
 * @details Update thread arguments and status then `pthread_cond_signal()` to
 * alert the corresponding thread of its new work
 */
void wake_up_thread(struct worker_list *list, int thread_index,
		int client_fd, struct timespec client_ts)
{
	struct worker *thread;

	thread = &(list->workers[thread_index]);
	/* ensure that the worker thread's status is correct before continuing */
	assert(thread->status == THREAD_READY);

	/* thread already exists: update status and arguments */
	pthread_mutex_lock(&thread->args.lock);

	/* set new client file descriptor */
	thread->args.client_fd = client_fd;

	/* update status fields (per-thread and global) */
	thread->status = THREAD_BUSY;
	thread->args.self_status = &thread->status;
	pthread_mutex_lock(&list->threads_status.lock);
	set_bit(&list->threads_status.data, thread_index, true);
	pthread_mutex_unlock(&list->threads_status.lock);

	/* update timestamp */
	thread->args.timestamp = client_ts;

	/* signal to thread that there's new work */
	pthread_cond_signal(&thread->args.cond);
	pthread_mutex_unlock(&thread->args.lock);
}
