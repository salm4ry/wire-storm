/// @file

#include <pthread.h>
#include <time.h>

#include "bitmask.h"

#define THREAD_AVAILABLE 0  ///< Thread has not yet been created
#define THREAD_BUSY 1 ///< Thread is working
#define THREAD_READY 2 ///< Thread has been created and is not working

struct status_mask {
	uint64_t data;
	pthread_mutex_t lock;
};

struct worker_args {
	int client_fd;
	int thread_index;
	struct timespec timestamp;
	pthread_mutex_t lock;
	pthread_cond_t cond;
	int *self_status;
	struct status_mask *threads_status;
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
