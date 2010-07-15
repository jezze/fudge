#include <lib/types.h>
#include <lib/io.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/screen.h>

screen_t screen;

vfs_node_t vfsScreen;

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

void screen_puts_num(unsigned int n, unsigned int base)
{

    if (!n)
    {

        screen_putc('0');
        return;

    }

    char s[32] = {0};

    int i;

    for (i = 30; n && i; --i, n /= base)
        s[i] = "0123456789abcdef"[n % base];

    screen_puts(s + i + 1);

}

void screen_puts_dec(unsigned int n)
{

    return screen_puts_num(n, 10);

}

void screen_puts_hex(unsigned int n)
{

    return screen_puts_num(n, 16);

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

unsigned int screen_write(vfs_node_t *node, unsigned int offset, unsigned int size, char *buffer)
{

    screen_puts(buffer);

    return size;

}

void screen_init()
{

    screen.address = (uint16_t *)SCREEN_ADDRESS;
    screen.cursorX = 0;
    screen.cursorY = 0;

    screen_set_text_color(SCREEN_COLOR_WHITE, SCREEN_COLOR_BLACK);

    string_copy(vfsScreen.name, "stdout");
    vfsScreen.inode = 0;
    vfsScreen.flags = VFS_FILE;
    vfsScreen.length = 0;
    vfsScreen.open = 0;
    vfsScreen.close = 0;
    vfsScreen.read = 0;
    vfsScreen.write = screen_write;
    vfsScreen.walk = 0;

    screen_clear();

}

