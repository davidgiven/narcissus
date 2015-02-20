#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

struct chord
{
	uint32_t keys;
	int cost;
};

static struct chord chords[256];
static int numchords = 0;

static int chord_compare_cb(const void* o1, const void* o2)
{
	const struct chord* s1 = o1;
	const struct chord* s2 = o2;

	if (s1->cost < s2->cost)
		return -1;
	else if (s1->cost > s2->cost)
		return 1;
	return 0;
}

static int calculate_cost(uint32_t chord)
{
	/* Single keys */

	switch (chord)
	{
		case 1<<3:
		case 1<<4:
		case 1<<5:
		case 1<<8:
		case 1<<9:
		case 1<<10:
			return 1;

		case 1<<1:
		case 1<<2:
		case 1<<6:
		case 1<<7:
			return 2;
	}

	int cost = 0;

	/* Sum the cost of all keys. */

	for (int i=1; i<=10; i++)
		if (chord & (1<<i))
			cost += calculate_cost(1<<i);

	/* If split between rows, penalise. */

	if ((chord & 0x003e) && (chord & 0x07c0))
		cost++;

	return cost;
}

static void addchord(uint32_t chord)
{
	chords[numchords].keys = chord;
	chords[numchords].cost = calculate_cost(chord);
	numchords++;
}

int main(int argc, const char* argv[])
{
	for (uint32_t i = 1; i<(1<<10); i++)
	{
		if (__builtin_popcount(i) <= 3)
			addchord(i << 1);
	}
	
	qsort(chords, numchords, sizeof(*chords), chord_compare_cb);

	for (int i=0; i<numchords; i++)
		printf("/* %d cost=%d */ %08x,\n", i, chords[i].cost, chords[i].keys);

	printf("Total number of chords: %d\n", numchords);
}

