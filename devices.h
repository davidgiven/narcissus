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

#endif

