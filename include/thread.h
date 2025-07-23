/// @file

#include <pthread.h>

#define THREAD_AVAILABLE 0  ///< Thread has not yet been created
#define THREAD_BUSY 1 ///< Thread is working
#define THREAD_READY 2 ///< Thread has been created and is not working

/* TODO fix naming */

struct worker_args {
	int client_fd;
	pthread_mutex_t lock;
	pthread_cond_t cond;
	int *status;
};

struct worker {
	pthread_t thread;
	int status;
	struct worker_args args;
};

void init_workers(struct worker **workers, int num_workers);
int find_thread(struct worker **workers, int num_workers);
