/* Narcissus
 * © 2015 David Given
 * This file is redistributable under the terms of the two-clause BSD license;
 * see COPYING in the distribution root for the full text.
 */

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
			allwords = w;
			count++;
		}
	}

	fclose(fp);

	wordlist = calloc(count+1, sizeof(*wordlist));
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

static bool isvalid(char c)
{
	for (int i=0; i<level; i++)
		if (c == levels[i])
			return true;
	return false;
}

static bool isvalidword(const char* s)
{
	while (*s)
	{
		if (!isvalid(*s))
			return false;
		s++;
	}
	return true;
}

static uint32_t djb2(const char* s)
{
    uint32_t hash = 5381;
    int c;

    while (c = *s++)
        hash = ((hash << 5) + hash) + (uint8_t)c; /* hash * 33 + c */

    return hash;
}

static int random_comparator_cb(const void* o1, const void* o2)
{
	uint32_t h1 = djb2(o1);
	uint32_t h2 = djb2(o2);

	if (h1 < h2)
		return -1;
	else if (h1 > h2)
		return 1;
	return 0;
}

static bool calculate_wordlist(void)
{
	struct word* w = allwords;
	int count = 0;

	while (w)
	{
		if (isvalidword(w->word))
		{
			wordlist[count] = w->word;
			count++;
		}
		w = w->next;
	}

	printf("(%d words at level %d)\n", count, level);
	if (count == 0)
		return false;

	qsort(wordlist, count, sizeof(*wordlist), random_comparator_cb);
	wordlist[count] = NULL;

	return true;
}

static bool askuser(const char* s)
{
	printf("Type %s: ", s);
	fflush(stdout);

	char c;
	for (int i=0; i<strlen(s); i++)
	{
		c = getchar();
		if (c != s[i])
			goto failed;
	}
	c = getchar();
	if (c != '\n')
		goto failed;
	return true;

failed:
	while (c != '\n')
		c = getchar();
	return false;
}

static void show_chords(void)
{
	printf("\nChords for level %d:\n", level);
	const struct chord* chord = device->chords;
	while (chord->buttons)
	{
		if (isvalid(chord->keysym))
		{
			printf("%c: ", chord->keysym);
			for (int i=0; i<32; i++)
			{
				if (chord->buttons & (1<<i))
					printf("%d ", i);
			}
			printf("\n");
		}
		chord++;
	}
}

int main(int argc, char* argv[])
{
	parse_options(argc, argv);

	find_device();
	open_dictionary();
	calculate_levels();

	for (;;)
	{
		if (!calculate_wordlist())
		{
			if (level == 26)
				error("couldn't find any words for level 26; giving up");

			fprintf(stderr, "(skipping level %d because the wordlist is empty!)\n",
				level);
			level++;
			continue;
		}

		show_chords();

		const char** s = wordlist;
		while (*s)
		{
			if (!askuser(*s))
			{
				printf("\nIncorrect!\n");
				continue;
			}

			s++;
		}

		level++;
		printf("\n(word list complete, advancing to level %d)\n", level);
	}
}

