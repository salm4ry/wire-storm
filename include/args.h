/// @file

#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>

/* TODO decide on reasonable defaults, minimum, and maximum values */
#define DEFAULT_NUM_WORKERS 30 ///< default number of client worker threads
#define DEFAULT_GRACE_PERIOD 1 ///< default grace period in seconds

#define MIN_NUM_WORKERS 1
#define MAX_NUM_WORKERS 50

#define MIN_GRACE_PERIOD 0
#define MAX_GRACE_PERIOD 5

/**
 * @brief Command-line arguments
 */
struct args {
	bool extended;  ///< use extended CTMP?
	int num_workers;  ///< number of client worker threads
	int grace_period; ///< grace period in seconds
};

void usage(char *prog_name);
void set_default_args(struct args *args);
void parse_args(int argc, char *argv[], struct args *args);
