/* Narcissus
 * © 2015 David Given
 * This file is redistributable under the terms of the two-clause BSD license;
 * see COPYING in the distribution root for the full text.
 */

/* This is a really dumb tool which calculates chords in order of increasing
 * cost, based on a set of cost heuristics I made up on a whim. It's probably
 * adaptable to other devices; I don't know whether it's worth it.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define HEX__(n) 0x##n##LU
#define B8__(x) ((x&0x0000000FLU)?1:0) \
	+((x&0x000000F0LU)?2:0) \
	+((x&0x00000F00LU)?4:0) \
	+((x&0x0000F000LU)?8:0) \
	+((x&0x000F0000LU)?16:0) \
	+((x&0x00F00000LU)?32:0) \
	+((x&0x0F000000LU)?64:0) \
	+((x&0xF0000000LU)?128:0)

#define B8(d) ((unsigned char)B8__(HEX__(d)))
#define B16(dmsb,dlsb) (((unsigned short)B8(dmsb)<<8) + B8(dlsb))
#define B32(dmsb,db2,db3,dlsb) \
	(((unsigned long)B8(dmsb)<<24) \
	+ ((unsigned long)B8(db2)<<16) \
	+ ((unsigned long)B8(db3)<<8) \
	+ B8(dlsb))

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

	/* Penalise some awkward three-letter chords. */

	switch (chord)
	{
		case B8(00110010):
		case B8(00100110):
		case B8(00101010):
			cost += 3;
			break;
	}

	return cost;
}

static void addchord(uint32_t chord)
{
	chords[numchords].keys = chord;
	chords[numchords].cost = calculate_cost(chord);
	numchords++;
}

static const char* tobinary(uint32_t chord)
{
	static char buffer[12];

	for (int i=1; i<=5; i++)
		buffer[i-1] = (chord & (1<<i)) ? '1' : '0';
	buffer[5] = ',';
	for (int i=6; i<=10; i++)
		buffer[i] = (chord & (1<<i)) ? '1' : '0';
	buffer[11] = '\0';

	return buffer;
}

int main(int argc, const char* argv[])
{
	for (uint32_t i = 1; i<(1<<5); i++)
	{
		if (__builtin_popcount(i) <= 3)
			addchord(i << 1);
	}
	addchord(B8(00011110));
	addchord(B8(00111100));
	
	qsort(chords, numchords, sizeof(*chords), chord_compare_cb);

	for (int i=0; i<numchords; i++)
		printf("[CHORD(%s)] = %d, /* cost=%d */\n",
			tobinary(chords[i].keys), i,
			chords[i].cost);

	printf("Total number of chords: %d\n", numchords);
}

