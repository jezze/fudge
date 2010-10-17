#include <lib/call.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <arch/x86/kernel/io.h>
#include <arch/x86/kernel/screen.h>
#include <arch/x86/kernel/vga.h>

struct vfs_node screenNode;
struct vfs_node *screenVgaNode;
struct vfs_node *screenVgaColorNode;
struct vfs_node *screenVgaCursorNode;
unsigned short screenOffset;
unsigned char screenColor;

static void screen_scroll()
{

    char buffer[2000];

    vfs_read(screenVgaNode, 80, 1920, buffer);
    memory_set(buffer + 1920, ' ', 80);
    vfs_write(screenVgaNode, 0, 2000, buffer);

    screenOffset -= SCREEN_CHARACTER_WIDTH;

}

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

        vfs_write(screenVgaNode, screenOffset, 1, &c);
        screenOffset++;

    }

    if (screenOffset >= SCREEN_CHARACTER_WIDTH * SCREEN_CHARACTER_HEIGHT - SCREEN_CHARACTER_WIDTH)
        screen_scroll();

    vfs_write(screenVgaCursorNode, 0, 1, &screenOffset);

}

static void screen_clear()
{

    char c = ' ';
    int i;

    for (i = 0; i < 2000; i++)
        vfs_write(screenVgaNode, i, 1, &c);

}

static unsigned int screen_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned int j = 0;

    for (i = offset; i < offset + count; i++, j++)
        screen_putc(((char *)buffer)[j]);

    return count;

}

void screen_init()
{

    screenVgaNode = call_open("dev/vga_fb");
    screenVgaColorNode = call_open("dev/vga_fb_color");
    screenVgaCursorNode = call_open("dev/vga_fb_cursor");

    unsigned char color = (SCREEN_COLOR_BLACK << 4) | (SCREEN_COLOR_WHITE & 0x0F);
    vfs_write(screenVgaColorNode, 0, 1, &color);

    screen_clear();

    memory_set(&screenNode, 0, sizeof (struct vfs_node));
    string_copy(screenNode.name, "stdout");
    screenNode.length = 2000;
    screenNode.write = screen_write;

    struct vfs_node *node = call_open("dev");
    vfs_write(node, node->length, 1, &screenNode);

}

