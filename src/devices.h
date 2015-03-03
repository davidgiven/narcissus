/* Narcissus
 * © 2015 David Given
 * This file is redistributable under the terms of the two-clause BSD license;
 * see COPYING in the distribution root for the full text.
 */

#ifndef DEVICES_H
#define DEVICES_H

#include <X11/Xlib.h>

struct button
{
	int keycode;
	uint32_t button;
};

struct chord
{
	uint32_t buttons;
	int keysym;
};

struct device
{
	const char* name;
	struct button* buttons;
	struct chord* chords;
};

extern const struct device razer_nostromo;

extern const struct device* find_connected_device(Display* display, int* deviceid);
extern const struct device* find_device_by_name(const char* name);
extern void load_device(const struct device* device);
uint32_t keycode_to_button(int keysym);
int decode_chord(uint32_t buttons);

#define MODIFIER_MASK ((1<<24) - 1)
#define CTRL          (1<<24)
#define ALT           (1<<25)
/* Note: META doesn't work with libfakekey 0.1-8.1 */
#define META          (1<<26)

#endif

