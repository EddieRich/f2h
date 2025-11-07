#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

struct s_options
{
	int skip_lines;
	int multiline;
	int header_guard;
	char var_name[BUFSIZ];
	char file_name[BUFSIZ];
} options = { 0 };

void show_options()
{
	fprintf(stderr, "options:\n");
	fprintf(stderr, "  skip %d lines\n", options.skip_lines);
	fprintf(stderr, "  multiline = %d\n", options.multiline);
	fprintf(stderr, "  header guard = %d\n", options.header_guard);
	fprintf(stderr, "  variable name = %s\n", options.var_name);
	fprintf(stderr, "  input filename = %s\n", options.file_name);
	fprintf(stderr, "\n");
}

void show_usage(char* appname)
{
	if (appname[0] == '.' && appname[1] == '/')
		appname += 2;

	fprintf(stderr, "\nusage: %s [options] filename\noptions:\n", appname);
	fprintf(stderr, "  -l n     skip n lines (0)\n");
	fprintf(stderr, "  -m       multiline, outut each line on it's own line (false)\n");
	fprintf(stderr, "  -h       include a header guard (false)\n");
	fprintf(stderr, "  -n name  variable name (filename no extension)\n");
	fprintf(stderr, "\n\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
	int opt;
	memset(options.var_name, 0, BUFSIZ);
	memset(options.file_name, 0, BUFSIZ);

	while ((opt = getopt(argc, argv, "l:n:mh")) != -1)
	{
		// ascii value of option letter
		switch (opt)
		{
		case 'l':
			// l has a mandatory value pointed to by optarg
			options.skip_lines = atoi(optarg);
			break;

		case 'n':
			// n has a mandatory value pointed to by optarg
			strcpy(options.var_name, optarg);
			break;

		case 'm':
			// m is a flag
			options.multiline = 1;
			break;

		case 'h':
			// h is a flag
			options.header_guard = 1;
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
		fprintf(stderr, "ERROR : file not specified\n");
		show_usage(argv[0]);
	}

	// full path and filename char* is now in argv[optind]
	strcpy(options.file_name, argv[optind]);
	if (!strlen(options.var_name))
	{
		char* start = strrchr(options.file_name, '/');

		if (!start)
			start = options.file_name;
		else
			start++;

		char* end = strchr(start, '.');
		if (!end)
			end = strchr(options.file_name, '\0');

		strncpy(options.var_name, start, end - start);
	}

	FILE* fp = fopen(options.file_name, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "\nERROR: Could not open file %s\n\n", options.file_name);
		exit(EXIT_FAILURE);
	}

	// ??? output a header guard
	if (options.header_guard)
	{
		printf("#ifndef __%s_H__\n", options.var_name);
		printf("#define __%s_H__\n\n", options.var_name);
	}

	char buf[BUFSIZ];
	// skip lines
	for (int lc = options.skip_lines; lc; lc--)
		fgets(buf, BUFSIZ, fp);

		// output the variable
	printf("const char* %s = ", options.var_name);
	if (options.multiline)
		printf("\n");
	else
		printf("\"");

	int bigfile = 0;
	while (!feof(fp))
	{
		memset(buf, 0, BUFSIZ);
		fread(buf, 1, BUFSIZ, fp);

		char* ptr = buf;
		if (!bigfile && options.multiline)
			putchar('"');

		int startline = 0;
		while (*ptr)
		{
			if (*ptr == '\n')
			{
				printf("\\n");

				if (options.multiline)
				{
					if (ptr[1] == '\0')
					{
						printf("\\0\";");
					}
					else
					{
						printf("\"");
						if (ptr[1] == '\0')
							printf(";");

						printf("\n");
						startline = 1;
					}
				}
				else
				{
					if (ptr[1] == '\0')
						printf("\";\n");
				}
			}
			else
			{
				if (startline)
				{
					putchar('"');
					startline = 0;
				}

				putchar(*ptr);
			}

			ptr++;
		}

		if (!feof(fp) && (ptr - buf) >= (BUFSIZ - 2))
			bigfile = 1;
	}

	fclose(fp);

	// ??? output a header guard
	if (options.header_guard)
	{
		printf("\n\n#endif // __%s_H__\n", options.var_name);
	}

	exit(EXIT_SUCCESS);
}

