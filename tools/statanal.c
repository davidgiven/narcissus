/* Narcissus
 * © 2015 David Given
 * This file is redistributable under the terms of the two-clause BSD license;
 * see COPYING in the distribution root for the full text.
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

struct letter
{
	int code;
	int frequency;
};

static struct letter frequencies[256];

static int letter_compare_cb(const void* o1, const void* o2)
{
	const struct letter* s1 = o1;
	const struct letter* s2 = o2;

	if (s1->frequency < s2->frequency)
		return 1;
	else if (s1->frequency > s2->frequency)
		return -1;
	return 0;
}

int main(int argc, const char* argv[])
{
	for (int i=0; i<(sizeof(frequencies)/sizeof(*frequencies)); i++)
	{
		frequencies[i].code = i;
		frequencies[i].frequency = 0;
	}

	for (;;)
	{
		int c = getchar();
		if (c == -1)
			break;

		frequencies[(uint8_t) c].frequency++;
	}

	qsort(frequencies, sizeof(frequencies)/sizeof(*frequencies),
		sizeof(*frequencies), letter_compare_cb);
	for (int i=0; i<(sizeof(frequencies)/sizeof(*frequencies)); i++)
	{
		if (frequencies[i].frequency > 0)
			printf("% 4d (%c): %d\n", frequencies[i].code, frequencies[i].code,
				frequencies[i].frequency);
	}
}

