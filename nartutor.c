#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include "devices.h"

static int level = 1;
static const char* filename = "/usr/share/dict/words";
static const char* devicename = NULL;

struct word
{
	const char* word;
	struct word* next;
};

static const struct device* device;
static struct word* allwords = NULL;
static const char** wordlist;
static char levels[26];

static void error(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);

	fprintf(stderr, "nartutor: ");
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");

	va_end(ap);
	exit(EXIT_FAILURE);
}

static void do_help(void)
{
	fprintf(stderr, "nartutor [-f dictionary] [-l level] [-d device]\n");
	exit(EXIT_SUCCESS);
}

static void parse_options(int argc, char* argv[])
{
	argv[0] = "nartutor";
	opterr = 0;

	for (;;)
	{
		switch (getopt(argc, argv, "-l:f:d:h"))
		{
			case 'l':
				level = atoi(optarg);
				if (level <= 0)
					error("invalid level (try 1 and go up)");
				break;

			case 'f':
				filename = optarg;
				break;

			case 'd':
				devicename = optarg;
				break;

			case 'h':
				do_help();
				assert(false);

			case '?':
				error("unknown option '%c' --- try -h", optopt);
				assert(false);

			case 1:
				error("too many command line arguments --- try -h");
				assert(false);

			case -1:
				return;
		}
	}
}

static void find_device(void)
{
	if (devicename)
	{
		device = find_device_by_name(devicename);
		if (!device)
			error("unrecognised device name");
	}
	else
	{
		Display* display = XOpenDisplay(NULL);
		device = find_connected_device(display, NULL);
		XCloseDisplay(display);
		if (!device)
			error("could not find a supported device");
	}
}

static bool validate_word(char* buffer)
{
	if ((*buffer == '\n') || (*buffer == '\0'))
		return false;

	for (;;)
	{
		if (!*buffer)
			return true;
		if (*buffer == '\n')
		{
			*buffer = '\0';
			return true;
		}
		if (!islower(*buffer))
			return false;
		buffer++;
	}
}

static void open_dictionary(void)
{
	FILE* fp = fopen(filename, "r");
	if (!fp)
		error("could not open dictionary: %s", strerror(errno));

	int count = 0;
	while (!feof(fp))
	{
		char buffer[80];
		fgets(buffer, sizeof(buffer), fp);

		if (validate_word(buffer))
		{
			struct word* w = calloc(1, sizeof(struct word));
			w->word = strdup(buffer);
			w->next = allwords;
			count++;
		}
	}

	fclose(fp);

	printf("(%d words in word list)\n", count);
}

static void calculate_levels(void)
{
	int level = 0;
	const struct chord* chord = device->chords;
	while (chord->buttons)
	{
		if (islower(chord->keysym))
		{
			levels[level] = chord->keysym;
			level++;
		}
		chord++;
	}

	assert(level == 26);
	printf("(learning order: %26s)\n", levels);
}

int main(int argc, char* argv[])
{
	parse_options(argc, argv);

	find_device();
	open_dictionary();
	calculate_levels();
}

