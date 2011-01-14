#include <lib/call.h>
#include <lib/elf.h>
#include <lib/memory.h>
#include <lib/file.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/kernel.h>
#include <kernel/vfs.h>
#include <modules/elf/elf.h>
#include <modules/tty//tty.h>

struct vfs_node *ttyVgaNode;
struct vfs_node *ttyVgaColorNode;
struct vfs_node *ttyVgaCursorNode;
unsigned short ttyVgaCursorOffset;

char ttyLocation[256];

static void tty_scroll()
{

    char buffer[TTY_CHARACTER_SIZE];

    file_read(ttyVgaNode, TTY_CHARACTER_WIDTH, TTY_CHARACTER_SIZE - TTY_CHARACTER_WIDTH, buffer);
    memory_set(buffer + TTY_CHARACTER_SIZE - TTY_CHARACTER_WIDTH, ' ', TTY_CHARACTER_WIDTH);
    file_write(ttyVgaNode, 0, TTY_CHARACTER_SIZE, buffer);

    ttyVgaCursorOffset -= TTY_CHARACTER_WIDTH;

}

static void tty_putc(char c)
{

    if (c == '\b')
    {

        ttyVgaCursorOffset--;

    }

    else if (c == '\t')
    {

        ttyVgaCursorOffset = (ttyVgaCursorOffset + 8) & ~(8 - 1);

    }

    else if (c == '\r')
    {

        ttyVgaCursorOffset -= (ttyVgaCursorOffset % TTY_CHARACTER_WIDTH);

    }

    else if (c == '\n')
    {

        ttyVgaCursorOffset += TTY_CHARACTER_WIDTH - (ttyVgaCursorOffset % TTY_CHARACTER_WIDTH);

    }
    
    else if (c >= ' ')
    {

        file_write(ttyVgaNode, ttyVgaCursorOffset, 1, &c);
        ttyVgaCursorOffset++;

    }

    if (ttyVgaCursorOffset >= TTY_CHARACTER_WIDTH * TTY_CHARACTER_HEIGHT - TTY_CHARACTER_WIDTH)
        tty_scroll();

}

static void tty_vga_clear()
{

    char c = ' ';
    int i;

    for (i = 0; i < TTY_CHARACTER_SIZE; i++)
        file_write(ttyVgaNode, i, 1, &c);

}

static unsigned int tty_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned int j = 0;

    for (i = offset; i < offset + count; i++, j++)
        tty_putc(((char *)buffer)[j]);

    file_write(ttyVgaCursorNode, 0, 1, &ttyVgaCursorOffset);

    return count;

}

static unsigned int tty_location_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    count = string_length(ttyLocation) - offset;

    string_copy(buffer, ttyLocation + offset);

    return count;

}

static unsigned int tty_location_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    count = string_length(ttyLocation) - offset;

    string_copy(ttyLocation + offset, buffer);

    return count;

}

static void tty_init_vga()
{

    ttyVgaNode = call_open("/dev/vga_fb");
    ttyVgaColorNode = call_open("/dev/vga_fb_color");
    ttyVgaCursorNode = call_open("/dev/vga_fb_cursor");

    unsigned char color = (TTY_COLOR_BLACK << 4) | (TTY_COLOR_WHITE & 0x0F);
    file_write(ttyVgaColorNode, 0, 1, &color);

    tty_vga_clear();

}

void tty_init()
{

    tty_init_vga();

    string_copy(ttyLocation, "/");

    struct vfs_node *ttyStdoutNode = vfs_add_node("tty", TTY_CHARACTER_SIZE);
    ttyStdoutNode->write = tty_write;

    struct vfs_node *ttyLocationNode = vfs_add_node("location", 256);
    ttyLocationNode->read = tty_location_read;
    ttyLocationNode->write = tty_location_write;

    struct vfs_node *devNode = call_open("/dev");
    file_write(devNode, devNode->length, 1, ttyStdoutNode);
    file_write(devNode, devNode->length, 1, ttyLocationNode);

}

