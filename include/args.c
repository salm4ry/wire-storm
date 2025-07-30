/// @file

#include <stdio.h>
#include <libgen.h>

#include "args.h"
#include "log.h"

static char *short_opts = "ehn:b:";  ///< short option characters

/**
 * @brief Long options
 * @details Each option defined as: `{const char *name, int has_arg, int *flag, int val}`
 */
static struct option long_opts[]  = {
	{"help", no_argument, NULL, 'h'},
	{"extended", no_argument, NULL, 'e'},
	{"num-workers", required_argument, NULL, 'n'},
	{"backlog", required_argument, NULL, 'b'},
	/* terminate option list with zeroed-struct */
	{NULL, 0, NULL, 0}
};

/**
 * @brief Print program usage
 * @param prog_name name of executable (`argv[0]`)
 */
void usage(char *prog_name)
{
	printf("usage: %s [OPTIONS]\n"
	       "-e, --extended: use extended CTMP\n"
	       "-n, --num-workers <NUM>: maximum number of client worker threads to use\n"
	       "-b, --backlog <LEN>: backlog length for listen(2) \n"
	       "-h, --help: print this message and exit\n", basename(prog_name));
}

/**
 * @brief Set default command-line argument values
 * @param args argument structure to set values of
 */
void set_default_args(struct args *args)
{
	args->extended = false;
	args->num_workers = DEFAULT_NUM_WORKERS;
	args->backlog = DEFAULT_BACKLOG;
}

/**
 * @brief Parse command-line arguments
 * @param argc argument count
 * @param argv argument vector
 * @param args argument structure to store results
 * @details Validate integer arguments (`num_workers` and `grace_period`) by
 * checking that they fall in the range of minimum and maximum values defined in
 * `args.h`
 */
void parse_args(int argc, char *argv[], struct args *args)
{
	int opt = 0, option_index = 0, tmp = 0;

	while ((opt = getopt_long(argc,  argv, short_opts, long_opts, &option_index)) != -1) {
		switch (opt) {
		case 'h':
			usage(argv[0]);
			exit(EXIT_SUCCESS);
		case 'e':
			args->extended = true;
			break;
		case 'n':
			tmp = atoi(optarg);
			if (tmp >= MIN_NUM_WORKERS && tmp <= MAX_NUM_WORKERS) {
				args->num_workers = tmp;
			} else {
				pr_err("invalid number of workers %d: (must be between %d and %d)\n",
						tmp, MIN_NUM_WORKERS, MAX_NUM_WORKERS);
				exit(EXIT_FAILURE);
			}
			break;
		case 'b':
			tmp = atoi(optarg);
			if (tmp >= MIN_BACKLOG && tmp <= MAX_BACKLOG) {
				args->backlog = tmp;
			} else {
				pr_err("invalid backlog %d (must be between %d and %d)\n",
						tmp, MIN_BACKLOG, MAX_BACKLOG);
				exit(EXIT_FAILURE);
			}
			break;
		default:
			/* invalid argument: print usage and exit */
			usage(argv[0]);
			exit(EXIT_FAILURE);
		}
	}
}
