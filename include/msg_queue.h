/// @file

#include <stdbool.h>
#include <time.h>
#include <sys/queue.h>

struct msg_entry {
	struct timespec timestamp;
	struct ctmp_msg *msg;  ///< CTMP message structure to broadcast
	bool *sent;   ///< TODO docstring
	TAILQ_ENTRY(msg_entry) entries;  ///< prev + next pointers for queue
};
TAILQ_HEAD(msg_queue, msg_entry);

void init_msg_entry(struct msg_entry **entry, struct ctmp_msg *msg,
		int num_threads);
void free_msg_entry(struct msg_entry *entry);

/* TODO better function names */
bool within_grace_period(struct msg_entry *entry, int grace_period);
bool can_forward(struct msg_entry *entry, struct timespec recv_start,
		int grace_period);
