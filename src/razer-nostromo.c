/* Narcissus
 * © 2015 David Given
 * This file is redistributable under the terms of the two-clause BSD license;
 * see COPYING in the distribution root for the full text.
 */

#include <stdlib.h>
#include <stdint.h>
#include <X11/keysym.h>
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

#define LETTER(lo, caps) \
	{ TOP5(HEX(lo)), caps | 32 }, \
	{ BOT5(HEX(lo)), caps }, \
	{ TOP5(HEX(lo))|B0, caps | 32 | CTRL }, \
	{ BOT5(HEX(lo))|B0, caps | 32 | ALT }

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
		/* Letters, sorted by frequency; most common first. */

		LETTER(00100, 'E'),
		LETTER(00010, 'T'),
		LETTER(00001, 'A'),
		LETTER(10000, 'O'),
		LETTER(01000, 'I'),
		LETTER(00110, 'N'),
		LETTER(00011, 'S'),
		LETTER(01100, 'H'),
		LETTER(00101, 'R'),
		LETTER(10100, 'D'),
		LETTER(10010, 'L'),
		LETTER(01010, 'U'),
		LETTER(10001, 'C'),
		LETTER(01001, 'M'),
		LETTER(00111, 'F'),
		LETTER(11000, 'G'),
		LETTER(10110, 'Y'),
		LETTER(01110, 'P'),
		LETTER(01101, 'W'),
		LETTER(01011, 'B'),
		LETTER(11100, 'V'),
		LETTER(11010, 'K'),
		LETTER(01111, 'X'),
		LETTER(11110, 'J'),
		LETTER(10101, 'Q'),
		LETTER(10011, 'Z'),

		/* Numbers. */

		{ B11|B1,    '1' },
		{ B11|B2,    '2' },
		{ B11|B3,    '3' },
		{ B11|B4,    '4' },
		{ B11|B5,    '5' },
		{ B11|B6,    '6' },
		{ B11|B7,    '7' },
		{ B11|B8,    '8' },
		{ B11|B9,    '9' },
		{ B11|B10,   '0' },

		{ B14,       XK_Escape },
		{ B11|B14,   '.' },

		/* Adjacent diagonals. */

		{ B8|B4,     XK_Return },
		{ B4|B10,    XK_BackSpace},
		{ B5|B9,     ' ' },

		/* Vertical pairs. */

		{ B1|B6,     '"' },
		{ B2|B7,     ';' },
		{ B3|B8,     ',' },
		{ B4|B9,     '=' },
		{ B5|B10,    '_' },

		/* Paired symbols, using the left column as a modifier. */

		{ B1|B10,    '(' },
		{ B6|B5,     ')' },
		{ B1|B9,     '{' },
		{ B6|B4,     '}' },
		{ B1|B8,     '[' },
		{ B6|B3,     ']' },
		{ B1|B7,     '<' },
		{ B6|B2,     '>' },

		/* Column 2. */

		{ B2|B9,     '+' },
		{ B2|B10,    '-' },
		{ B7|B4,     '*' },
		{ B7|B5,     '/' },
		{ B2|B8|B9,  '%' },
		{ B7|B3|B4,  '?' },

		/* Column 3. */

		{ B3|B6,     '|' },
		{ B3|B10,    ':' },
		{ B8|B1,     '~' },
		{ B8|B5,     '&' },
		{ B3|B6|B10, '\\' },
		{ B8|B1|B5,  '`' },

		/* Column 4. */

		{ B4|B6,     '#' },
		{ B4|B7,     '^' },
		{ B9|B1,     '!' },
		{ B10|B2,    '$' },
		{ B4|B7|B8,  '@' },
		{ B9|B2|B3,  XK_sterling },

		/* Cursor keys! */

		{ B16,     XK_Up },
		{ B17,     XK_Right },
		{ B18,     XK_Down },
		{ B19,     XK_Left },

		{ B16|B11, XK_Page_Up },
		{ B17|B11, XK_End },
		{ B18|B11, XK_Page_Down },
		{ B19|B11, XK_Home },

		{ 0, 0 }
	}
};

