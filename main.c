#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

struct s_options
{
	int skip_lines;
	int multiline;
	int binary;
	int header_guard;
	char var_name[64];
	char var_type[64];
	char file_name[BUFSIZ];
} options = { 0 };

void show_options()
{
	fprintf(stderr, "options:\n");
	fprintf(stderr, "  skip %d lines\n", options.skip_lines);
	fprintf(stderr, "  multiline = %d\n", options.multiline);
	fprintf(stderr, "  binary = %d\n", options.binary);
	fprintf(stderr, "  header guard = %d\n", options.header_guard);
	fprintf(stderr, "  variable name = %s\n", options.var_name);
	fprintf(stderr, "  variable type = %s\n", options.var_type);
	fprintf(stderr, "  input filename = %s\n", options.file_name);
	fprintf(stderr, "\n");
}

void show_usage(char* appname)
{
	if (appname[0] == '.' && appname[1] == '/')
		appname += 2;

	fprintf(stderr, "\nusage: %s [options] filename\noptions:\n", appname);
	fprintf(stderr, "  -ln      skip n lines of ASCII text (0)\n");
	fprintf(stderr, "  -m       multiline, outut each line on it's own line (false)\n");
	fprintf(stderr, "  -b[c]    output an array of binary bytes in [c] columns (16)\n");
	fprintf(stderr, "  -h       include a header guard (false)\n");
	fprintf(stderr, "  -n name  variable name (filename no extension)\n");
	fprintf(stderr, "  -t type  variable type (const char *)\n");
	fprintf(stderr, "\n\n");
	exit(EXIT_FAILURE);
}

char buf[BUFSIZ];

void print_ascii(FILE* fp)
{
	printf("%s %s = ", options.var_type, options.var_name);
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
}

void print_binary(FILE* fp)
{
	printf("%s %s[] = {\n", options.var_type, options.var_name);

	int total_bytes = 0;
	int col = 0;
	while (!feof(fp))
	{
		int bytes = fread(buf, 1, BUFSIZ, fp);
		if (!bytes)
			break;

		total_bytes += bytes;
		char* ptr = buf;

		while (bytes--)
		{
			printf("0x%02X", (unsigned char)(*ptr++));

			if (bytes || !feof(fp))
				putchar(',');

			if (++col == options.binary)
			{
				putchar('\n');
				col = 0;
			}
		}
	}

	printf("};\nconst size_t sz%s = %d;", options.var_name, total_bytes);
}

int main(int argc, char* argv[])
{
	int opt;
	memset(options.var_name, 0, sizeof(options.var_name));
	memset(options.var_type, 0, sizeof(options.var_type));
	memset(options.file_name, 0, BUFSIZ);

	while ((opt = getopt(argc, argv, "l:n:t:mb::h")) != -1)
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

		case 't':
			// t has a mandatory value pointed to by optarg
			strcpy(options.var_type, optarg);
			break;

		case 'm':
			// m is a flag
			options.multiline = 1;
			break;

		case 'b':
			// b has an optional value
			options.binary = 16;
			if (optarg)
				options.binary = atoi(optarg);

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

	if (!strlen(options.var_type))
	{
		if (options.binary)
			strcpy(options.var_type, "const char");
		else
			strcpy(options.var_type, "const char*");
	}

	// ??? output a header guard
	if (options.header_guard)
	{
		printf("#ifndef __%s_H__\n", options.var_name);
		printf("#define __%s_H__\n\n", options.var_name);
	}

	FILE* fp = fopen(options.file_name, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "\nERROR: Could not open file %s\n\n", options.file_name);
		exit(EXIT_FAILURE);
	}

	// skip lines
	for (int lc = options.skip_lines; lc; lc--)
	{
		fgets(buf, BUFSIZ, fp);
		if (options.binary) // show the skipped lines as comments
			printf("// %s", buf);
	}

	if (options.binary)
		print_binary(fp);
	else
		print_ascii(fp);

	fclose(fp);

	printf("\n");

	// ??? output a header guard
	if (options.header_guard)
	{
		printf("\n#endif // __%s_H__\n", options.var_name);
	}

	exit(EXIT_SUCCESS);
}

