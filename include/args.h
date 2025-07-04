#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>

struct args {
	bool extended;
};

static char *short_opts = "eh";  ///< short option characters

static struct option long_opts[]  = {
	{"help", no_argument, NULL, 'h'},
	{"extended", no_argument, NULL, 'e'},
	/* terminate option list with zeroed-struct */
	{NULL, 0, NULL, 0}
};

void usage(char *prog_name);
void set_default_args(struct args *args);
void parse_args(int argc, char *argv[], struct args *args);
