#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>

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

static uint32_t pressed = 0;
static Time presstime = 0;

#define LETTER(lo, caps) \
	[CHORD5__(HEX__(lo)) << 1] = caps | 32, \
	[CHORD5__(HEX__(lo)) << 6] = caps
	
#define NUMBER(num) \
	[(1<<num) | 1] = '0' + num

static uint8_t chorddecodetable[1<<11] = {
	LETTER(00100, 'E'),
	LETTER(00010, 'I'),
	LETTER(00001, 'S'),
	LETTER(10000, 'A'),
	LETTER(01000, 'R'),
	LETTER(00110, 'N'),
	LETTER(00101, 'T'),
	LETTER(00011, 'O'),
	LETTER(10100, 'L'),
	LETTER(01100, 'C'),
	LETTER(10010, 'D'),
	LETTER(01010, 'U'),
	LETTER(10001, 'G'),
	LETTER(01001, 'P'),
	LETTER(00111, 'M'),
	LETTER(11000, 'H'),
	LETTER(10110, 'B'),
	LETTER(01110, 'Y'),
	LETTER(01101, 'F'),
	LETTER(01011, 'V'),
	LETTER(11100, 'K'),
	LETTER(11010, 'W'),
	LETTER(01111, 'Z'),
	LETTER(11110, 'X'),
	LETTER(10101, 'J'),
	LETTER(10011, 'Q'),

	NUMBER(1),
	NUMBER(2),
	NUMBER(3),
	NUMBER(4),
	NUMBER(5),
	NUMBER(6),
	NUMBER(7),
	NUMBER(8),
	NUMBER(9),
	[(1<<10) | 1] = '0',
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
	{
		printf("new chord!\n");
		presstime = time;
	}

	int decoded = 0;
	if (pressed < (1<<11))
		decoded = chorddecodetable[pressed];
	if (!decoded)
		decoded = '?';

	printf("%08X %d '%c'\n", pressed, time - presstime, decoded);
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
				printf("event %d\n", event.type);
		}
	}
	return 0;
}

