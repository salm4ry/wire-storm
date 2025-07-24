/// @file

#include <sys/queue.h>
#include <stdbool.h>

struct msg_entry {
	struct ctmp_msg *msg;  ///< CTMP message structure to broadcast
	bool *sent;   ///< TODO docstring
	TAILQ_ENTRY(msg_entry) entries;  ///< prev + next pointers for queue
};
TAILQ_HEAD(msg_queue, msg_entry);

void init_msg_entry(struct msg_entry **entry, struct ctmp_msg *msg,
		int num_threads);
void free_msg_entry(struct msg_entry *entry);
