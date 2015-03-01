/* Narcissus
 * © 2015 David Given
 * This file is redistributable under the terms of the two-clause BSD license;
 * see COPYING in the distribution root for the full text.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
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
static int xi_opcode;
static int pipefds[2];

static uint32_t pressed = 0;
static bool transmitpending = true;

static void error(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);

	fprintf(stderr, "narcissus: ");
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");

	va_end(ap);
	exit(EXIT_FAILURE);
}

static void sigalrm_handler_cb(int signum)
{
	/* Write a single byte to the pipe to wake up the event loop. */
	uint8_t c = 0;
	write(pipefds[1], &c, 1);
}

static void settimer(int milliseconds)
{
	struct itimerval it = {
		.it_interval = {0, 0},
		.it_value = {0, milliseconds*1000}
	};
	setitimer(ITIMER_REAL, &it, NULL);
}

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
	if (s)
		error("unable to grab device (narcissus may already be running)");

	s = XISelectEvents(display, window, &eventmask, 1);
	assert(s == 0);
}

static void send_chord_key(void)
{
	if (transmitpending && pressed)
	{
		int decoded = decode_chord(pressed);
		if (decoded)
		{
			int modifiers = 0;
			if (decoded & CTRL)
				modifiers |= FAKEKEYMOD_CONTROL;
			if (decoded & ALT)
				modifiers |= FAKEKEYMOD_ALT;
			if (decoded & META)
				modifiers |= FAKEKEYMOD_META;

			fakekey_press_keysym(fakekey, decoded & MODIFIER_MASK, modifiers);
			transmitpending = false;
		}
	}
}

static void change_state(int keycode, Time time, bool state)
{
	uint32_t mask = keycode_to_button(keycode);
	if (!mask)
		return;

	if (state)
	{
		uint32_t old = pressed;
		pressed |= mask;

		/* If the chord changed, send a timer event in 100ms. */

		if (pressed != old)
			settimer(100);
	}
	else
	{
		/* Stop the timer. */

		settimer(0);

		/* Make sure that any partially completed character is sent. */

		send_chord_key();

		/* Send a keyup. */

		fakekey_release(fakekey);
		pressed &= ~mask;
	}

	/* If the current pressed chord no longer maps to a valid key, assume
	 * the current keypress has finished. We allow the user to hold down
	 * chords which don't map to keys to make modifiers work properly. */

	if (!pressed || !decode_chord(pressed))
		transmitpending = true;
}

static void process_xlib_events(void)
{
	while (XPending(display))
	{
		XEvent event;
		XNextEvent(display, &event);

		switch (event.type)
		{
			case GenericEvent:
			{
				XGetEventData(display, &event.xcookie);
				if (event.xcookie.extension == xi_opcode)
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
}

static void process_timer_events(void)
{
	/* The signal handler wrote a single byte to the pipe; read it. */

	uint8_t c;
	read(pipefds[0], &c, 1);

	/* Decode the chord and send it. */

	send_chord_key();
}

static void emergency_key_release_cb(void)
{
	fakekey_release(fakekey);
}

int main(int argc, const char* argv[])
{
	/* Set up X */

	display = XOpenDisplay(NULL);

	int eventbase, errorbase;
	if (!XQueryExtension(display, "XInputExtension",
			&xi_opcode, &eventbase, &errorbase))
	{
		printf("X Input extension not available.\n");
		return EXIT_FAILURE;
	}

	window = XDefaultRootWindow(display);
	fakekey = fakekey_init(display);
	atexit(emergency_key_release_cb);

	/* Find and set up the hardware device */

	const struct device* device = find_connected_device(display, &deviceid);
	assert(device);
	load_device(device);

	/* Set up our timed event pipe */

	pipe(pipefds);
	struct sigaction sa;
	sa.sa_handler = sigalrm_handler_cb;
	sa.sa_flags = 0;
	sigfillset(&sa.sa_mask);
	sigaction(SIGALRM, &sa, NULL);

	/* Grab the device and enter the event loop. */

	grab_xinput_device();

	XSelectInput(display, window, GenericEvent);
	for(;;)
	{
		struct pollfd fds[2] = {
			{ 
				.fd = ConnectionNumber(display),
				.events = POLLIN,
				.revents = 0
			},
			{
				.fd = pipefds[0],
				.events = POLLIN,
				.revents = 0
			}
		};
		poll(fds, sizeof(fds)/sizeof(*fds), -1);

		if (fds[0].revents)
			process_xlib_events();
		if (fds[1].revents)
			process_timer_events();
	}

	return 0;
}

