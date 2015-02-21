#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "devices.h"

static int numbuttons;
static int numchords;

static struct button* buttons;
static struct chord* chords;

static int chord_comparator_cb(const void* o1, const void* o2)
{
	const struct chord* s1 = o1;
	const struct chord* s2 = o2;

	if (s1->buttons < s2->buttons)
		return -1;
	if (s1->buttons > s2->buttons)
		return 1;
	return 0;
}

static int button_comparator_cb(const void* o1, const void* o2)
{
	const struct button* s1 = o1;
	const struct button* s2 = o2;

	if (s1->keycode < s2->keycode)
		return -1;
	if (s1->keycode > s2->keycode)
		return 1;
	return 0;
}

const struct device* find_device(const char* name)
{
	if (strcmp(razer_nostromo.name, name) == 0)
		return &razer_nostromo;
	return NULL;
}

void load_device(const struct device* device)
{
	const struct button* b = device->buttons;
	numbuttons = 0;
	while (b->keycode)
	{
		numbuttons++;
		b++;
	}

	const struct chord* c = device->chords;
	numchords = 0;
	while (c->buttons)
	{
		numchords++;
		c++;
	}

	printf("%d buttons, %d chords\n", numbuttons, numchords);

	buttons = calloc(numbuttons, sizeof(struct button));
	memcpy(buttons, device->buttons, numbuttons * sizeof(struct button));
	qsort(buttons, numbuttons, sizeof(struct button), button_comparator_cb);

	chords = calloc(numchords, sizeof(struct chord));
	memcpy(chords, device->chords, numchords * sizeof(struct chord));
	qsort(chords, numchords, sizeof(struct chord), chord_comparator_cb);
}

uint32_t keycode_to_button(int keysym)
{
	const struct button key = { keysym, 0 };
	const struct button* b = bsearch(&key,
		buttons, numbuttons, sizeof(struct button), button_comparator_cb);

	if (b)
		return b->button;
	return 0;
}

int decode_chord(uint32_t buttons)
{
	const struct chord key = { buttons, 0 };
	const struct chord* c = bsearch(&key,
		chords, numchords, sizeof(struct chord), chord_comparator_cb);

	if (c)
		return c->keysym;
	return 0;
}

