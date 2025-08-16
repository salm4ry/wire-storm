/**
 * @file args.h
 * @brief Constants, structs, and functions relating to command-line arguments
 * @details Defines default, minimum, and maximum values
 */

#include <stdbool.h>
#include <getopt.h>

#define DEFAULT_EXTENDED false  ///< use original CTMP by default

#define MIN_NUM_WORKERS 1  ///< need at least one thread to handle receivers
#define MAX_NUM_WORKERS 64  ///< bounded by `sent` field `struct msg_entry`
#define DEFAULT_NUM_WORKERS 32 ///< default number of client worker threads

#define MIN_BACKLOG MIN_NUM_WORKERS
#define MAX_BACKLOG MAX_NUM_WORKERS
#define DEFAULT_BACKLOG DEFAULT_NUM_WORKERS  ///< default backlog for listen()

#define MIN_TTL 2
#define MAX_TTL 10
#define DEFAULT_TTL 5  ///< default time that messages remain in memory for

/**
 * @brief Command-line arguments
 */
struct args {
	bool extended;  ///< use extended CTMP?
	int num_workers;  ///< number of client worker threads
	int backlog;  //< backlog size for listen()
	int ttl;  ///< message time to live
};

void usage(char *prog_name);
void set_default_args(struct args *args);
bool valid_int_arg(int arg, int min, int max);
void pr_arg_err(const char *arg_name, int arg_val, int min, int max);
void parse_args(int argc, char *argv[], struct args *args);
