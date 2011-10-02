#include <lib/call.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <arch/x86/modules/vga/vga.h>
#include <modules/tty/tty.h>

static struct tty_device ttyDevice;
static struct tty_cwd_device ttyCwdDevice;

static void tty_scroll()
{

    char buffer[TTY_CHARACTER_SIZE];

    ttyDevice.vgaDevice->node.operations.read(&ttyDevice.vgaDevice->node, TTY_CHARACTER_WIDTH, TTY_CHARACTER_SIZE - TTY_CHARACTER_WIDTH, buffer);
    memory_set(buffer + TTY_CHARACTER_SIZE - TTY_CHARACTER_WIDTH, ' ', TTY_CHARACTER_WIDTH);
    ttyDevice.vgaDevice->node.operations.write(&ttyDevice.vgaDevice->node, 0, TTY_CHARACTER_SIZE, buffer);

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

        ttyDevice.vgaDevice->node.operations.write(&ttyDevice.vgaDevice->node, ttyDevice.cursorOffset, 1, &c);
        ttyDevice.cursorOffset++;

    }

    if (ttyDevice.cursorOffset >= TTY_CHARACTER_WIDTH * TTY_CHARACTER_HEIGHT)
        tty_scroll();

}

static void tty_vga_clear()
{

    char c = ' ';
    int i;

    for (i = 0; i < TTY_CHARACTER_SIZE; i++)
        ttyDevice.vgaDevice->node.operations.write(&ttyDevice.vgaDevice->node, i, 1, &c);

}

static void tty_set_color(unsigned char fg, unsigned char bg)
{

    ttyDevice.cursorColor = (bg << 4) | (fg & 0x0F);

}

static unsigned int tty_device_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned int j = 0;

    for (i = offset; i < offset + count; i++, j++)
        tty_putc(((char *)buffer)[j]);

    ttyDevice.vgaCursorNode->operations.write(ttyDevice.vgaCursorNode, 0, 1, &ttyDevice.cursorOffset);

    return count;

}

static unsigned int tty_cwd_device_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    count = string_length(ttyCwdDevice.path) - offset;

    string_copy(buffer, ttyCwdDevice.path + offset);

    return count;

}

static unsigned int tty_cwd_device_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    count = string_length(ttyCwdDevice.path) - offset;

    string_copy(ttyCwdDevice.path + offset, buffer);

    return count;

}

void tty_init()
{

    string_copy(ttyDevice.base.name, "tty");
    ttyDevice.base.node.length = TTY_CHARACTER_SIZE;
    ttyDevice.base.node.operations.write = tty_device_write;
    ttyDevice.cursorOffset = 0;
    ttyDevice.vgaDevice = modules_get_device(MODULES_DEVICE_TYPE_VGA);
    ttyDevice.vgaColorNode = vfs_find_root("/color");
    ttyDevice.vgaCursorNode = vfs_find_root("/cursor");
    ttyDevice.set_color = tty_set_color;
    ttyDevice.set_color(TTY_COLOR_WHITE, TTY_COLOR_BLACK);
    modules_register_device(MODULES_DEVICE_TYPE_KEYBOARD, &ttyDevice.base);

    string_copy(ttyCwdDevice.base.name, "cwd");
    ttyCwdDevice.base.node.length = TTY_CWD_SIZE;
    ttyCwdDevice.base.node.operations.read = tty_cwd_device_read;
    ttyCwdDevice.base.node.operations.write = tty_cwd_device_write;
    string_copy(ttyCwdDevice.path, "/");
    modules_register_device(MODULES_DEVICE_TYPE_KEYBOARD, &ttyCwdDevice.base);

    ttyDevice.vgaColorNode->operations.write(ttyDevice.vgaColorNode, 0, 1, &ttyDevice.cursorColor);

    tty_vga_clear();

}

