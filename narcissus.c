#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>

static Display* display;
static Window window;
static int device;

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

static void change_state(int keycode, Time time, bool state)
{
	printf("%d %ld %d\n", keycode, time, state);
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

