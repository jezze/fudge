#include <system.h>
#include <mem.h>
#include <screen.h>

uint16_t *textmemptr;
int32_t attrib = 0x0F;
int32_t csr_x = 0, csr_y = 0;

void putc(char c)
{

	uint16_t *where;
	uint32_t att = attrib << 8;

	if (c == 0x08)
	{

		if (csr_x != 0)
			csr_x--;

	}

	else if (c == 0x09)
	{

		csr_x = (csr_x + 8) & ~(8 - 1);

	}

	else if (c == '\r')
	{

		csr_x = 0;

	}

	else if (c == '\n')
	{

		csr_x = 0;
		csr_y++;

	}
	
	else if (c >= ' ')
	{

		where = textmemptr + (csr_y * 80 + csr_x);
		*where = c | att;
		csr_x++;

	}

	if (csr_x >= 80)
	{

		csr_x = 0;
		csr_y++;

	}

	screen_scroll();
	screen_move_cursor();

}

void puts(char *text)
{

	int i;

	for (i = 0; i < strlen(text); i++)
		putc(text[i]);

}

void puts_dec(uint32_t n)
{

	if (n == 0)
	{
		putc('0');
		return;
	}

	int32_t acc = n;
	char c[32];
	int i = 0;

	while (acc > 0)
	{

		c[i] = '0' + acc % 10;
		acc /= 10;
		i++;

	}

	c[i] = 0;

	char c2[32];
	c2[i--] = 0;
	int j = 0;

	while (i >= 0)
		c2[i--] = c[j++];

	puts(c2);

}

void puts_hex(uint32_t n)
{

	int32_t tmp;

	puts("0x");

	char noZeroes = 1;

	int i;

	for (i = 28; i > 0; i -= 4)
	{

		tmp = (n >> i) &0xF;

		if (tmp == 0 && noZeroes != 0)
			continue;

		if (tmp >= 0xA)
		{

			noZeroes = 0;
			putc(tmp - 0xA + 'a');

		}

		else
		{

			noZeroes = 0;
			putc(tmp + '0');

		}

	}

	tmp = n & 0xF;

	if (tmp >= 0xA)
		putc(tmp - 0xA + 'a');
	else
		putc(tmp + '0');

}

void screen_set_text_color(uint8_t forecolor, uint8_t backcolor)
{

	attrib = (backcolor << 4) | (forecolor & 0x0F);

}

void screen_clear()
{

	uint32_t blank;
	int32_t i;

	blank = 0x20 | (attrib << 8);

	for (i = 0; i < 25; i++)
		memsetw(textmemptr + i * 80, blank, 80);

	csr_x = 0;
	csr_y = 0;

	screen_move_cursor();

}

void screen_move_cursor()
{

	uint32_t temp;

	temp = csr_y * 80 + csr_x;

	outb(0x3D4, 14);
	outb(0x3D5, temp >> 8);
	outb(0x3D4, 15);
	outb(0x3D5, temp);

}

void screen_scroll()
{

	uint32_t blank, temp;

	blank = 0x20 | (attrib << 8);

	if (csr_y >= 25)
	{

		temp = csr_y - 25 + 1;
		memcpy(textmemptr, textmemptr + temp * 80, (25 - temp) * 80 * 2);
		memsetw(textmemptr + (25 - temp) * 80, blank, 80);
		csr_y = 25 - 1;

	}

}

void screen_init()
{

	textmemptr = (uint16_t *)ADDRESS_VIDEO;
	screen_clear();

}

