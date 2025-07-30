/// @file

#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>

#define DEFAULT_NUM_WORKERS 32 ///< default number of client worker threads
#define DEFAULT_BACKLOG 16     ///< default backlog for listen()

#define MIN_NUM_WORKERS 1  ///< need at least one thread to handle receivers
#define MAX_NUM_WORKERS 64  ///< bounded by `sent` field `struct msg_entry`

/* TODO change min/max backlog? */
#define MIN_BACKLOG MIN_NUM_WORKERS
#define MAX_BACKLOG MAX_NUM_WORKERS

/**
 * @brief Command-line arguments
 */
struct args {
	bool extended;  ///< use extended CTMP?
	int num_workers;  ///< number of client worker threads
	int backlog;  //< backlog size for listen()
};

void usage(char *prog_name);
void set_default_args(struct args *args);
void parse_args(int argc, char *argv[], struct args *args);
