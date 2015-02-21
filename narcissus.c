#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>
#include <fakekey/fakekey.h>

#define HEX__(n) 0x##n##LU

#define CHORD5__(x) \
	(  ((x&0xf0000LU) ? (1<<0) : 0) \
	 | ((x&0x0f000LU) ? (1<<1) : 0) \
	 | ((x&0x00f00LU) ? (1<<2) : 0) \
	 | ((x&0x000f0LU) ? (1<<3) : 0) \
	 | ((x&0x0000fLU) ? (1<<4) : 0))

static Display* display;
static Window window;
static int device;
static FakeKey* fakekey;

static uint32_t pressed = 0;
static bool previous = false;
static Time presstime = 0;

#define LETTER1(lo, caps) \
	[CHORD5__(HEX__(lo)) << 1] = caps | 32, \
	[CHORD5__(HEX__(lo)) << 6] = caps

#define LETTER3(lo, caps, loshift, hishift) \
	[CHORD5__(HEX__(lo)) << 1] = caps | 32, \
	[CHORD5__(HEX__(lo)) << 6] = caps, \
	[(CHORD5__(HEX__(lo)) << 1) | 1] = loshift, \
	[(CHORD5__(HEX__(lo)) << 6) | 1] = hishift
	
static uint8_t chorddecodetable[1<<11] = {
	/* Sorted by frequency; most common first. */
	LETTER3(00100, 'E', '3', '8'),
	LETTER3(00010, 'T', '4', '9'),
	LETTER3(00001, 'A', '5', '0'),
	LETTER3(10000, 'O', '1', '6'),
	LETTER3(01000, 'I', '2', '7'),
	LETTER3(00110, 'N', '(', ')'),
	LETTER3(00011, 'S', '.', ','),
	LETTER3(01100, 'H', ';', ':'),
	LETTER1(00101, 'R'),
	LETTER1(10100, 'D'),
	LETTER1(10010, 'L'),
	LETTER1(01010, 'U'),
	LETTER1(10001, 'C'),
	LETTER1(01001, 'M'),
	LETTER1(00111, 'F'),
	LETTER1(11000, 'G'),
	LETTER1(10110, 'Y'),
	LETTER1(01110, 'P'),
	LETTER1(01101, 'W'),
	LETTER1(01011, 'B'),
	LETTER1(11100, 'V'),
	LETTER1(11010, 'K'),
	LETTER1(01111, 'X'),
	LETTER1(11110, 'J'),
	LETTER1(10101, 'Q'),
	LETTER1(10011, 'Z'),
};

static void find_device(void)
{
	int num;
	XIDeviceInfo* devices = XIQueryDevice(display, XIAllDevices, &num);
	for (int i=0; i<num; i++)
	{
		XIDeviceInfo* info = &devices[i];

		bool is_keyboard = false;
		for (int j=0; j<info->num_classes; j++)
			is_keyboard = is_keyboard || (info->classes[j]->type == XIKeyClass);

		if (is_keyboard &&
			(strcmp(info->name, "Razer Razer Nostromo") == 0))
		{
			device = info->deviceid;
			return;
		}
	}

	assert(false);
}

static void grab_device(void)
{
	XIEventMask eventmask;
	uint8_t bitmask[2] = { 0, 0 };

	eventmask.deviceid = XIAllMasterDevices;
	eventmask.mask = (unsigned char*) bitmask;
	eventmask.mask_len = sizeof(bitmask);

	XISetMask(bitmask, XI_KeyPress);
	XISetMask(bitmask, XI_KeyRelease);

	Status s = XIGrabDevice(display, device, window,
			CurrentTime, None, XIGrabModeAsync, XIGrabModeAsync,
			False, &eventmask);
	assert(s == 0);

	s = XISelectEvents(display, window, &eventmask, 1);
	assert(s == 0);
}

static int decode_key(int keycode)
{
	switch (keycode)
	{
		case  23: return 1;
		case  24: return 2;
		case  25: return 3;
		case  26: return 4;
		case  27: return 5;

		case  66: return 6;
		case  38: return 7;
		case  39: return 8;
		case  40: return 9;
		case  41: return 10;

		case  50: return 11;
		case  52: return 12;
		case  53: return 13;
		case  54: return 14;

		case  65: return 15;
		case  64: return 0;
		case 111: return 16;
		case 114: return 17;
		case 116: return 18;
		case 113: return 19;
	}

	return -1;
}

static void change_state(int keycode, Time time, bool state)
{
	int key = decode_key(keycode);
	if (key == -1)
		return;

	uint32_t mask = 1U << key;
	uint32_t old = pressed;
	if (state)
		pressed |= mask;
	else
		pressed &= ~mask;
	
	if (pressed && !old)
		presstime = time;

	int decoded = 0;
	if (pressed < (1<<11))
		decoded = chorddecodetable[pressed];
	else
	{
		switch (pressed)
		{
			case (1<<13): decoded = ' '; break;
			case (1<<14): decoded = 127; break;
			case (1<<15): decoded = 13; break;
		}
	}

	if (state)
	{
		if (decoded)
		{
			if (previous)
			{
				fakekey_release(fakekey);
				fakekey_press_keysym(fakekey, 127, 0);
				fakekey_release(fakekey);
			}
			fakekey_press_keysym(fakekey, decoded, 0);
			previous = true;
		}
	}
	else
	{
		fakekey_release(fakekey);
		previous = false;
	}
}

int main(int argc, const char* argv[])
{
	display = XOpenDisplay(NULL);

	int opcode;
	int eventbase, errorbase;
	if (!XQueryExtension(display, "XInputExtension",
			&opcode, &eventbase, &errorbase))
	{
		printf("X Input extension not available.\n");
		return EXIT_FAILURE;
	}

	window = XDefaultRootWindow(display);
	fakekey = fakekey_init(display);
	find_device();
	grab_device();

	for(;;)
	{
		XEvent event;
		XNextEvent(display, &event);

		switch (event.type)
		{
			case GenericEvent:
			{
				XGetEventData(display, &event.xcookie);
				if (event.xcookie.extension == opcode)
				{
					switch (event.xcookie.evtype)
					{
						case XI_KeyPress:
						case XI_KeyRelease:
						{
							XIDeviceEvent* xie = event.xcookie.data;
							change_state(xie->detail, xie->time,
								(event.xcookie.evtype == XI_KeyPress));
							break;
						}
					}
				}
				break;
			}

			default:
				break;
		}
	}
	return 0;
}

