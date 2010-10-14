#include <lib/call.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <arch/x86/kernel/io.h>
#include <arch/x86/kernel/screen.h>

struct vfs_node screenNode;
unsigned int screenOffset;
unsigned short screenColor;

static void screen_putc(char c)
{

    if (c == '\b')
    {

        screenOffset--;

    }

    else if (c == '\t')
    {

        screenOffset = (screenOffset + 8) & ~(8 - 1);

    }

    else if (c == '\r')
    {

        screenOffset -= (screenOffset % SCREEN_CHARACTER_WIDTH);

    }

    else if (c == '\n')
    {

        screenOffset += SCREEN_CHARACTER_WIDTH - (screenOffset % SCREEN_CHARACTER_WIDTH);

    }
    
    else if (c >= ' ')
    {

        struct vfs_node *node = call_open("dev/vga");
        vfs_write(node, screenOffset, 1, &c);

        screenOffset++;

    }

    if (screenOffset >= SCREEN_CHARACTER_WIDTH * SCREEN_CHARACTER_HEIGHT - SCREEN_CHARACTER_WIDTH)
        screen_scroll();

    screen_cursor_move();

}

void screen_set_text_color(unsigned char forecolor, unsigned char backcolor)
{

    screenColor = (backcolor << 4) | (forecolor & 0x0F);

}

void screen_clear()
{

    unsigned short blank = ' ' | (screenColor << 8);

    memory_setw((void *)SCREEN_ADDRESS, blank, SCREEN_CHARACTER_WIDTH * SCREEN_CHARACTER_HEIGHT * 2);

    screenOffset = 0;

}

void screen_cursor_move()
{

    io_outb(0x3D4, 14);
    io_outb(0x3D5, screenOffset >> 8);
    io_outb(0x3D4, 15);
    io_outb(0x3D5, screenOffset);

}

void screen_scroll()
{

    char buffer[2000];

    struct vfs_node *node = call_open("dev/vga");
    vfs_read(node, 80, 1920, buffer);
    memory_set(buffer + 1920, ' ', 80);
    vfs_write(node, 0, 2000, buffer);

/*
    unsigned short blank = ' ' | (screenColor << 8);

    memory_copy((void *)SCREEN_ADDRESS, (void *)(SCREEN_ADDRESS + SCREEN_CHARACTER_WIDTH * 2), SCREEN_CHARACTER_WIDTH * SCREEN_CHARACTER_HEIGHT * 2 - SCREEN_CHARACTER_WIDTH * 2);
    memory_setw((void *)(SCREEN_CHARACTER_WIDTH * SCREEN_CHARACTER_HEIGHT * 2 - SCREEN_CHARACTER_WIDTH * 2), blank, SCREEN_CHARACTER_WIDTH);
*/

    screenOffset -= SCREEN_CHARACTER_WIDTH;

}

unsigned int screen_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned int j = 0;

    for (i = offset; i < offset + count; i++, j++)
        screen_putc(((char *)buffer)[j]);

    return count;

}

void screen_init()
{

    screen_set_text_color(SCREEN_COLOR_WHITE, SCREEN_COLOR_BLACK);

    memory_set(&screenNode, 0, sizeof (struct vfs_node));
    string_copy(screenNode.name, "stdout");
    screenNode.write = screen_write;

    struct vfs_node *node = call_open("dev");
    vfs_write(node, node->length, 1, &screenNode);

    screen_clear();

}

