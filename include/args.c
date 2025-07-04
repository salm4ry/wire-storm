#include <stdio.h>
#include <libgen.h>

#include "args.h"

void usage(char *prog_name)
{
	printf("usage: %s [OPTIONS]\n"
	       "-e, --extended: use extended CTMP\n"
	       "-h, --help: print this message and exit\n", basename(prog_name));
}

void set_default_args(struct args *args)
{
	args->extended = false;
}

void parse_args(int argc, char *argv[], struct args *args)
{
	int opt = 0, option_index = 0;

	while ((opt = getopt_long(argc,  argv, short_opts, long_opts, &option_index)) != -1) {
		switch (opt) {
		case 'h':
			usage(argv[0]);
			exit(EXIT_SUCCESS);
		case 'e':
			args->extended = true;
			break;
		default:
			/* invalid argument: print usage and exit */
			usage(argv[0]);
			exit(EXIT_FAILURE);
		}
	}
}
