#include <system.h>
#include <mem.h>
#include <irq.h>
#include <kbd.h>

char kbdMapLowerUS[128] =
{
	0, 27,
	'1', '2', '3', '4', '5', '6', '7', '8',	'9', '0', '-', '=', '\b', '\t',
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
	0,
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	0,
	'\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
	0,
	'*',
	0,
	' ',
	0,
	0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0,
	0,
	0,
	0,
	0,
	0,
	'-',
	0,
	0,
	0,
	'+',
	0,
	0,
	0,
	0,
	0,
	0, 0, 0,
	0,
	0,
	0,
};

char kbdMapUpperUS[128] =
{
	0, 27,
	'!', ' ', '#', ' ', '%', '&', '/', '(',	')', '=', '-', '=', '\b', '\t',
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',
	0,
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',
	0,
	'\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/',
	0,
	'*',
	0,
	' ',
	0,
	0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0,
	0,
	0,
	0,
	0,
	0,
	'-',
	0,
	0,
	0,
	'+',
	0,
	0,
	0,
	0,
	0,
	0, 0, 0,
	0,
	0,
	0,
};

char kbdBuffer[KBD_BUFFER_SIZE];
uint16_t kbdBufferSize = 0;

uint8_t kbdToggleShift = 0;
uint8_t kbdToggleCtrl = 0;
uint8_t kbdToggleAlt = 0;

void kbd_buffer_clear()
{

	kbdBufferSize = 0;

}

void kbd_buffer_write(char c)
{

	if (kbdBufferSize < KBD_BUFFER_SIZE)
	{

		kbdBuffer[kbdBufferSize] = c;
		kbdBufferSize++;

	}

}

char kbd_buffer_read()
{

	if (kbdBufferSize)
	{

		kbdBufferSize--;
		return kbdBuffer[kbdBufferSize];

	}

	return 0;

}

void kbd_handler(registers_t *r)
{

	uint8_t scancode;

	scancode = inb(0x60);

	if (scancode == 0x2A)
		kbdToggleShift = 1;

	if (scancode == 0xAA)
		kbdToggleShift = 0;

	if (scancode & 0x80)
	{

		// Break codes

	}

	else
	{

		// Make codes
		if (kbdToggleShift)
			kbd_buffer_write(kbdMapUpperUS[scancode]);
		else
			kbd_buffer_write(kbdMapLowerUS[scancode]);

	}

}

void kbd_init()
{

	irq_register_handler(1, kbd_handler);

}

