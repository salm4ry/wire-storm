/**
 * @file thread.h
 * @brief Constants, structs, and functions for handling worker threads
 * @details Used by the destination server to handle concurrent clients
 */

#include <pthread.h>
#include <time.h>

#include "bitmask.h"

#define THREAD_AVAILABLE 0  ///< Thread has not yet been created
#define THREAD_BUSY 1 ///< Thread is working
#define THREAD_READY 2 ///< Thread has been created and is not working

/**
 * @brief Bitmask for storing status of all worker threads
 */
struct status_mask {
	uint64_t data;
	pthread_mutex_t lock;
};

/**
 * @brief Worker thread arguments
 * @details `threads_status` uses binary idle/busy (0 = idle, 1 = busy), whereas
 * `self_status` has three possible values (available, busy, and ready)
 */
struct worker_args {
	int client_fd;  ///< file descriptor to send messages to
	int thread_index;  ///< thread's own index
	struct timespec timestamp;  ///< time the client was accepted
	pthread_mutex_t lock;
	pthread_cond_t cond;
	int *self_status;  ///< own thread status (pointer to `worker` status)
	struct status_mask *threads_status;  ///< status of all worker threads (pointer to `worker_list` threads_status)
};

struct worker {
	pthread_t thread;
	int status;
	struct worker_args args;
};

struct worker_list {
	struct status_mask threads_status;
	int num_workers;
	struct worker *workers;
};

void init_workers(struct worker_list *list, int num_workers);
int find_idle_thread(struct worker_list *workers);

void init_thread_info(struct worker_list *list, int thread_index,
		int client_fd, struct timespec client_ts);
void wake_up_thread(struct worker_list *list, int thread_index,
		int client_fd, struct timespec client_ts);
