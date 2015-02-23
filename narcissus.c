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
#include "devices.h"

static Display* display;
static Window window;
static int deviceid;
static FakeKey* fakekey;

static uint32_t pressed = 0;
static bool previous = false;
static Time presstime = 0;

static void grab_xinput_device(void)
{
	XIEventMask eventmask;
	uint8_t bitmask[2] = { 0, 0 };

	eventmask.deviceid = XIAllMasterDevices;
	eventmask.mask = (unsigned char*) bitmask;
	eventmask.mask_len = sizeof(bitmask);

	XISetMask(bitmask, XI_KeyPress);
	XISetMask(bitmask, XI_KeyRelease);

	Status s = XIGrabDevice(display, deviceid, window,
			CurrentTime, None, XIGrabModeAsync, XIGrabModeAsync,
			False, &eventmask);
	assert(s == 0);

	s = XISelectEvents(display, window, &eventmask, 1);
	assert(s == 0);
}

static void change_state(int keycode, Time time, bool state)
{
	uint32_t mask = keycode_to_button(keycode);
	if (!mask)
		return;

	uint32_t old = pressed;
	if (state)
		pressed |= mask;
	else
		pressed &= ~mask;
	
	if (pressed && !old)
		presstime = time;

	if (state)
	{
		int decoded = decode_chord(pressed);
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

	const struct device* device = find_connected_device(display, &deviceid);
	assert(device);
	load_device(device);

	grab_xinput_device();

	XSelectInput(display, window, GenericEvent);
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

