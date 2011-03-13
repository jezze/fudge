#include <lib/call.h>
#include <lib/memory.h>
#include <lib/file.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/tty/tty.h>

struct modules_tty_device ttyDevice;

struct vfs_node *ttyVgaNode;
struct vfs_node *ttyVgaColorNode;
struct vfs_node *ttyVgaCursorNode;

char ttyCwd[256];

static void tty_scroll()
{

    char buffer[TTY_CHARACTER_SIZE];

    file_read(ttyVgaNode, TTY_CHARACTER_WIDTH, TTY_CHARACTER_SIZE - TTY_CHARACTER_WIDTH, buffer);
    memory_set(buffer + TTY_CHARACTER_SIZE - TTY_CHARACTER_WIDTH, ' ', TTY_CHARACTER_WIDTH);
    file_write(ttyVgaNode, 0, TTY_CHARACTER_SIZE, buffer);

    ttyDevice.cursorOffset -= TTY_CHARACTER_WIDTH;

}

static void tty_putc(char c)
{

    if (c == '\b')
    {

        ttyDevice.cursorOffset--;

    }

    else if (c == '\t')
    {

        ttyDevice.cursorOffset = (ttyDevice.cursorOffset + 8) & ~(8 - 1);

    }

    else if (c == '\r')
    {

        ttyDevice.cursorOffset -= (ttyDevice.cursorOffset % TTY_CHARACTER_WIDTH);

    }

    else if (c == '\n')
    {

        ttyDevice.cursorOffset += TTY_CHARACTER_WIDTH - (ttyDevice.cursorOffset % TTY_CHARACTER_WIDTH);

    }
    
    else if (c >= ' ')
    {

        file_write(ttyVgaNode, ttyDevice.cursorOffset, 1, &c);
        ttyDevice.cursorOffset++;

    }

    if (ttyDevice.cursorOffset >= TTY_CHARACTER_WIDTH * TTY_CHARACTER_HEIGHT - TTY_CHARACTER_WIDTH)
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

    file_write(ttyVgaCursorNode, 0, 1, &ttyDevice.cursorOffset);

    return count;

}

static unsigned int tty_cwd_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    count = string_length(ttyCwd) - offset;

    string_copy(buffer, ttyCwd + offset);

    return count;

}

static unsigned int tty_cwd_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    count = string_length(ttyCwd) - offset;

    string_copy(ttyCwd + offset, buffer);

    return count;

}

static void tty_set_color(unsigned char fg, unsigned char bg)
{

    ttyDevice.cursorColor = (bg << 4) | (fg & 0x0F);

}

static void tty_init_vga()
{

    ttyDevice.cursorOffset = 0;
    ttyDevice.set_color = tty_set_color;
    ttyDevice.set_color(TTY_COLOR_WHITE, TTY_COLOR_BLACK);

    modules_set_tty_device(&ttyDevice);

    ttyVgaNode = call_open("/dev/vga_fb");
    ttyVgaColorNode = call_open("/dev/vga_fb_color");
    ttyVgaCursorNode = call_open("/dev/vga_fb_cursor");

    file_write(ttyVgaColorNode, 0, 1, &ttyDevice.cursorColor);

    tty_vga_clear();

}

void tty_init()
{

    tty_init_vga();

    string_copy(ttyCwd, "/");

    struct vfs_node *ttyStdoutNode = vfs_add_node("tty", TTY_CHARACTER_SIZE);
    ttyStdoutNode->write = tty_write;

    struct vfs_node *ttyCwdNode = vfs_add_node("cwd", 256);
    ttyCwdNode->read = tty_cwd_read;
    ttyCwdNode->write = tty_cwd_write;

    struct vfs_node *devNode = call_open("/dev");
    file_write(devNode, devNode->length, 1, ttyStdoutNode);
    file_write(devNode, devNode->length, 1, ttyCwdNode);

}

