#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

struct s_options
{
	int verbose;
} options = { 0 };

void show_options()
{
	printf("options:\n");
	printf("  verbose=%d\n", options.verbose);
}

void show_usage(char* appname)
{
	if (appname[0] == '.' && appname[1] == '/')
		appname += 2;

	printf("\nusage: %s [options]\noptions:\n", appname);
	printf("  -v       verbose, show lots of information\n");
	printf("\n\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "a:b::v")) != -1)
	{
		// ascii value of option letter
		switch (opt)
		{
		case 'a':
			// a has a mandatory value pointed to by optarg
			// options.a = atoi(optarg);
			break;

		case 'b':
			// b has an optional value pointed to by optarg
			// options.b = b_default_value
			// if (optarg != NULL)
			//   options.b = atof(optarg);
			break;

		case 'v':
			// v is a flag
			options.verbose = 1;
			break;

		default:
			// unknown option
			show_usage(argv[0]);
		}
	}

	// argc and argv have been modified
	// if a filename (for example) should have been specified but wasn't,
	// optind will = argc, show the error
	if (optind >= argc)
	{
		printf("ERROR : file not specified\n");
		show_usage(argv[0]);
	}

	// filename is *argv[optind]


	exit(EXIT_SUCCESS);
}

