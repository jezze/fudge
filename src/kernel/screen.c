#include <lib/types.h>
#include <lib/mem.h>
#include <lib/string.h>
#include <kernel/screen.h>

screen_t screen;

void screen_putc(screen_t *screen, char c)
{

    uint16_t *where;

    if (c == '\b')
    {

        if (screen->cursorX != 0)
            screen->cursorX--;

    }

    else if (c == '\t')
    {

        screen->cursorX = (screen->cursorX + 8) & ~(8 - 1);

    }

    else if (c == '\r')
    {

        screen->cursorX = 0;

    }

    else if (c == '\n')
    {

        screen->cursorX = 0;
        screen->cursorY++;

    }
    
    else if (c >= ' ')
    {

        where = screen->address + (screen->cursorY * SCREEN_CHARACTER_WIDTH + screen->cursorX);
        *where = c | screen->context.attribute << 8;
        screen->cursorX++;

    }

    if (screen->cursorX >= SCREEN_CHARACTER_WIDTH)
    {

        screen->cursorX = 0;
        screen->cursorY++;

    }

    screen_scroll(screen);
    screen_cursor_move(screen);

}

void screen_puts(screen_t *screen, char *s)
{

    uint32_t i;

    for (i = 0; s[i] != '\0'; i++)
        screen_putc(screen, s[i]);

}

void screen_puts_dec(screen_t *screen, uint32_t n)
{

    if (n == 0)
    {

        screen_putc(screen, '0');
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

    screen_puts(screen, c2);

}

void screen_puts_hex(screen_t *screen, uint32_t n)
{

    int32_t tmp;

    screen_puts(screen, "0x");

    char noZeroes = 1;

    uint32_t i;

    for (i = 28; i > 0; i -= 4)
    {

        tmp = (n >> i) &0xF;

        if (tmp == 0 && noZeroes != 0)
            continue;

        if (tmp >= 0xA)
        {

            noZeroes = 0;
            screen_putc(screen, tmp - 0xA + 'a');

        }

        else
        {

            noZeroes = 0;
            screen_putc(screen, tmp + '0');

        }

    }

    tmp = n & 0xF;

    if (tmp >= 0xA)
        screen_putc(screen, tmp - 0xA + 'a');
    else
        screen_putc(screen, tmp + '0');

}

void screen_set_text_color(screen_t *screen, uint8_t forecolor, uint8_t backcolor)
{

    screen->context.attribute = (backcolor << 4) | (forecolor & 0x0F);

}

void screen_clear(screen_t *screen)
{

    uint32_t blank;

    blank = 0x20 | (screen->context.attribute << 8);

    uint32_t i;

    for (i = 0; i < SCREEN_CHARACTER_HEIGHT; i++)
        memsetw(screen->address + i * SCREEN_CHARACTER_WIDTH, blank, SCREEN_CHARACTER_WIDTH);

    screen->cursorX = 0;
    screen->cursorY = 0;

    screen_cursor_move(screen);

}

void screen_cursor_move(screen_t *screen)
{

    uint32_t temp;

    temp = screen->cursorY * SCREEN_CHARACTER_WIDTH + screen->cursorX;

    outb(0x3D4, 14);
    outb(0x3D5, temp >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, temp);

}

void screen_scroll(screen_t *screen)
{

    uint32_t blank, temp;

    blank = 0x20 | (screen->context.attribute << 8);

    if (screen->cursorY >= SCREEN_CHARACTER_HEIGHT)
    {

        temp = screen->cursorY - SCREEN_CHARACTER_HEIGHT + 1;
        memcpy(screen->address, screen->address + temp * SCREEN_CHARACTER_WIDTH, (SCREEN_CHARACTER_HEIGHT - temp) * SCREEN_CHARACTER_WIDTH * 2);
        memsetw(screen->address + (SCREEN_CHARACTER_HEIGHT - temp) * SCREEN_CHARACTER_WIDTH, blank, SCREEN_CHARACTER_WIDTH);
        screen->cursorY = SCREEN_CHARACTER_HEIGHT - 1;

    }

}

void screen_init()
{

    screen_context_t context;
    context.attribute = 0x0F;

    screen.address = (uint16_t *)SCREEN_ADDRESS;
    screen.cursorX = 0;
    screen.cursorY = 0;
    screen.context = context;

    screen_clear(&screen);

}

