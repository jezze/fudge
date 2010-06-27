#include <lib/types.h>
#include <lib/io.h>
#include <lib/memory.h>
#include <kernel/screen.h>

screen_t screen;

void screen_putc(char c)
{

    uint16_t *where;

    if (c == '\b')
    {

        if (screen.cursorX != 0)
            screen.cursorX--;

    }

    else if (c == '\t')
    {

        screen.cursorX = (screen.cursorX + 8) & ~(8 - 1);

    }

    else if (c == '\r')
    {

        screen.cursorX = 0;

    }

    else if (c == '\n')
    {

        screen.cursorX = 0;
        screen.cursorY++;

    }
    
    else if (c >= ' ')
    {

        where = screen.address + (screen.cursorY * SCREEN_CHARACTER_WIDTH + screen.cursorX);
        *where = c | screen.context.attribute << 8;
        screen.cursorX++;

    }

    if (screen.cursorX >= SCREEN_CHARACTER_WIDTH)
    {

        screen.cursorX = 0;
        screen.cursorY++;

    }

    screen_scroll();
    screen_cursor_move();

}

void screen_puts(char *s)
{

    unsigned int i;

    for (i = 0; s[i] != '\0'; i++)
        screen_putc(s[i]);

}

void screen_puts_dec(unsigned int n)
{

    if (n == 0)
    {

        screen_putc('0');
        return;

    }

    unsigned int acc = n;
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

    screen_puts(c2);

}

void screen_puts_hex(unsigned int n)
{

    int tmp;
    unsigned char noZeroes = 1;
    unsigned int i;

    for (i = 28; i > 0; i -= 4)
    {

        tmp = (n >> i) & 0xF;

        if (tmp == 0 && noZeroes != 0)
            continue;

        if (tmp >= 0xA)
        {

            noZeroes = 0;
            screen_putc(tmp - 0xA + 'a');

        }

        else
        {

            noZeroes = 0;
            screen_putc(tmp + '0');

        }

    }

    tmp = n & 0xF;

    if (tmp >= 0xA)
        screen_putc(tmp - 0xA + 'a');
    else
        screen_putc(tmp + '0');

}

void screen_puts_bcd(unsigned char n)
{

    screen_puts_dec(n >> 4);
    screen_puts_dec(n & 0x0F);

}

void screen_set_text_color(uint8_t forecolor, uint8_t backcolor)
{

    screen.context.attribute = (backcolor << 4) | (forecolor & 0x0F);

}

void screen_clear()
{

    uint32_t blank;

    blank = 0x20 | (screen.context.attribute << 8);

    unsigned int i;

    for (i = 0; i < SCREEN_CHARACTER_HEIGHT; i++)
        memory_setw(screen.address + i * SCREEN_CHARACTER_WIDTH, blank, SCREEN_CHARACTER_WIDTH);

    screen.cursorX = 0;
    screen.cursorY = 0;

    screen_cursor_move();

}

void screen_cursor_move()
{

    uint32_t temp;

    temp = screen.cursorY * SCREEN_CHARACTER_WIDTH + screen.cursorX;

    outb(0x3D4, 14);
    outb(0x3D5, temp >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, temp);

}

void screen_scroll()
{

    uint32_t blank, temp;

    blank = 0x20 | (screen.context.attribute << 8);

    if (screen.cursorY >= SCREEN_CHARACTER_HEIGHT)
    {

        temp = screen.cursorY - SCREEN_CHARACTER_HEIGHT + 1;
        memory_copy(screen.address, screen.address + temp * SCREEN_CHARACTER_WIDTH, (SCREEN_CHARACTER_HEIGHT - temp) * SCREEN_CHARACTER_WIDTH * 2);
        memory_setw(screen.address + (SCREEN_CHARACTER_HEIGHT - temp) * SCREEN_CHARACTER_WIDTH, blank, SCREEN_CHARACTER_WIDTH);
        screen.cursorY = SCREEN_CHARACTER_HEIGHT - 1;

    }

}

void screen_init()
{

    screen.address = (uint16_t *)SCREEN_ADDRESS;
    screen.cursorX = 0;
    screen.cursorY = 0;

    screen_set_text_color(SCREEN_COLOR_WHITE, SCREEN_COLOR_BLACK);

    screen_clear();

}

