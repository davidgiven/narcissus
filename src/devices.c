/* Narcissus
 * © 2015 David Given
 * This file is redistributable under the terms of the two-clause BSD license;
 * see COPYING in the distribution root for the full text.
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>
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

const struct device* find_connected_device(Display* display, int* deviceid)
{
	int num;
	XIDeviceInfo* devices = XIQueryDevice(display, XIAllDevices, &num);
	for (int i=0; i<num; i++)
	{
		XIDeviceInfo* info = &devices[i];

		bool is_keyboard = false;
		for (int j=0; j<info->num_classes; j++)
			is_keyboard = is_keyboard || (info->classes[j]->type == XIKeyClass);

		if (is_keyboard)
		{
			const struct device* device = find_device_by_name(info->name);
			if (device)
			{
				printf("Found %s\n", info->name);
				if (deviceid)
					*deviceid = info->deviceid;
				return device;
			}
		}
	}

	return NULL;
}

const struct device* find_device_by_name(const char* name)
{
	if (strcmp(razer_nostromo.name, name) == 0)
		return &razer_nostromo;
	return NULL;
}

static void print_chord(uint32_t chord)
{
	for (int i=0; i<32; i++)
		if (chord & (1<<i))
			printf("%d ", i);
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

	/* Check the chordmap for collisions. */

	for (int i=0; i<(numchords-1); i++)
	{
		struct chord* left = &chords[i];
		struct chord* right = &chords[i+1];
		if (left->buttons == right->buttons)
		{
			printf("warning: chord ");
			print_chord(left->buttons);
			printf("collides: keysyms '%s' and '%s'\n",
				XKeysymToString(left->keysym),
				XKeysymToString(right->keysym));
		}
	}
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

