#include <system.h>
#include <irq.h>
#include <screen.h>

uint8_t kbdus[128] =
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

uint8_t buffer[256];
uint16_t buffer_num = 0;

void keyboard_buffer_write(uint8_t c)
{

	if (buffer_num < 256)
	{

		buffer[buffer_num] = c;
		buffer_num++;

	}

}

uint8_t keyboard_buffer_read()
{

	uint8_t c;

	if (buffer_num > 0)
	{

		buffer_num--;
		c = buffer[buffer_num];

	}

	return c;

}

uint16_t keyboard_buffer_size()
{

	return buffer_num;

}

void keyboard_handler(registers_t *r)
{

	uint8_t scancode;

	scancode = inb(0x60);

	if (scancode & 0x80)
	{

	}

	else
	{

		keyboard_buffer_write(kbdus[scancode]);

//		putc(kbdus[scancode]);

	}

}

void keyboard_init()
{

	irq_register_handler(1, keyboard_handler);

}

