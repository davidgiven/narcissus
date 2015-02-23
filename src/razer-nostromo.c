#include <stdlib.h>
#include <stdint.h>
#include "devices.h"

#define B0 (1<<0)
#define B1 (1<<1)
#define B2 (1<<2)
#define B3 (1<<3)
#define B4 (1<<4)
#define B5 (1<<5)
#define B6 (1<<6)
#define B7 (1<<7)
#define B8 (1<<8)
#define B9 (1<<9)
#define B10 (1<<10)
#define B11 (1<<11)
#define B12 (1<<12)
#define B13 (1<<13)
#define B14 (1<<14)
#define B15 (1<<15)
#define B16 (1<<16)
#define B17 (1<<17)
#define B18 (1<<18)
#define B19 (1<<19)

#define HEX(n) 0x##n##LU

#define TOP5(x) \
	(  ((x&0xf0000LU) ? B1 : 0) \
	 | ((x&0x0f000LU) ? B2 : 0) \
	 | ((x&0x00f00LU) ? B3 : 0) \
	 | ((x&0x000f0LU) ? B4 : 0) \
	 | ((x&0x0000fLU) ? B5 : 0))

#define BOT5(x) \
	(  ((x&0xf0000LU) ? B6 : 0) \
	 | ((x&0x0f000LU) ? B7 : 0) \
	 | ((x&0x00f00LU) ? B8 : 0) \
	 | ((x&0x000f0LU) ? B9 : 0) \
	 | ((x&0x0000fLU) ? B10 : 0))

#define LETTER3(lo, caps, loshift, hishift) \
	{ TOP5(HEX(lo)), caps | 32 }, \
	{ BOT5(HEX(lo)), caps }, \
	{ TOP5(HEX(lo)) | B0, loshift }, \
	{ BOT5(HEX(lo)) | B0, hishift }

#define LETTER1(lo, caps) \
	{ TOP5(HEX(lo)), caps | 32 }, \
	{ BOT5(HEX(lo)), caps }

const struct device razer_nostromo = {
	/* sic --- this is how XInput2 reports it */
	.name = "Razer Razer Nostromo",

	/* Mapping of XInput2 keycode to the button mask */
	.buttons = (struct button[]) {
		{  23, B1 },
		{  24, B2 },
		{  25, B3 },
		{  26, B4 },
		{  27, B5 },

		{  66, B6 },
		{  38, B7 },
		{  39, B8 },
		{  40, B9 },
		{  41, B10 },

		{  50, B11 },
		{  52, B12 },
		{  53, B13 },
		{  54, B14 },

		{  65, B15 },
		{  64, B0 },
		{ 111, B16 },
		{ 114, B17 },
		{ 116, B18 },
		{ 113, B19 },

		{ 0, 0 }
	},

	/* Supported chords; sorted by frequency, most common first (or else
	 * the tutor gets confused). */
	.chords = (struct chord[]) {
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

		{ B12,    13 },
		{ B13,    ' ' },
		{ B13|B0, '\t' },
		{ B14,    127 },

		{ 0, 0 }
	}
};

