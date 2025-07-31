/// @file

#include <stdbool.h>
#include <time.h>
#include <sys/queue.h>
#include <pthread.h>

#include "bitmask.h"

struct msg_entry {
	struct timespec timestamp;
	struct ctmp_msg *msg;  ///< CTMP message structure to broadcast
	/**
	 * @brief bitmask representing which workers have sent this message
	 * @details 64 bits so max 64 workers at any time
	 */
	uint64_t sent;
	pthread_rwlock_t sent_lock;  ///< read/write lock for `sent`
	TAILQ_ENTRY(msg_entry) entries;  ///< prev + next pointers for queue
};
TAILQ_HEAD(msg_queue, msg_entry);

void init_msg_entry(struct msg_entry **entry, struct ctmp_msg *msg,
		int num_threads);
struct msg_entry *get_msg_entry(struct msg_queue *head, pthread_mutex_t *lock,
		pthread_cond_t *cond, struct msg_entry *current,
		struct msg_entry *prev, bool to_start);

bool is_sent(struct msg_entry *entry, int thread_index);
void update_sent(struct msg_entry *entry, int thread_index, bool val);
bool can_forward(struct msg_entry *entry, int thread_index,
		struct timespec recv_start);
