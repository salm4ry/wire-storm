/// @file

#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>

/**
 * @brief Command-line arguments
 */
struct args {
	bool extended;  ///< use extended CTMP?
};

void usage(char *prog_name);
void set_default_args(struct args *args);
void parse_args(int argc, char *argv[], struct args *args);
