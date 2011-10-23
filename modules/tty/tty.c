#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/vga/vga.h>
#include <modules/kbd/kbd.h>
#include <modules/stream/stream.h>
#include <modules/tty/tty.h>

static struct tty_device ttyDevice;

static void tty_clear()
{

    char c = ' ';
    int i;

    for (i = 0; i < TTY_CHARACTER_SIZE; i++)
        ttyDevice.vgaDevice->write_framebuffer(i, 1, &c);

}

static void tty_scroll()
{

    char buffer[TTY_CHARACTER_SIZE];

    ttyDevice.vgaDevice->read_framebuffer(TTY_CHARACTER_WIDTH, TTY_CHARACTER_SIZE - TTY_CHARACTER_WIDTH, buffer);
    memory_set(buffer + TTY_CHARACTER_SIZE - TTY_CHARACTER_WIDTH, ' ', TTY_CHARACTER_WIDTH);
    ttyDevice.vgaDevice->write_framebuffer(0, TTY_CHARACTER_SIZE, buffer);

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

        ttyDevice.vgaDevice->write_framebuffer(ttyDevice.cursorOffset, 1, &c);
        ttyDevice.cursorOffset++;

    }

    if (ttyDevice.cursorOffset >= TTY_CHARACTER_WIDTH * TTY_CHARACTER_HEIGHT)
        tty_scroll();

}

static unsigned int tty_device_node_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        char c;

        if (!ttyDevice.kbdDevice->buffer.getc(&ttyDevice.kbdDevice->buffer, &c))
            break;

        ((char *)buffer)[i] = c;

    }

    return i;

}

static unsigned int tty_device_node_write(struct vfs_node *self, unsigned int count, void *buffer)
{

    unsigned int i;

    for (i = 0; i < count; i++)
        tty_putc(((char *)buffer)[i]);

    ttyDevice.vgaDevice->set_cursor_offset(ttyDevice.cursorOffset);

    return count;

}

static unsigned int tty_cwd_device_node_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    count = string_length("/");

    string_copy(buffer, "/");

    return count;

}

static unsigned int tty_cwd_device_node_write(struct vfs_node *self, unsigned int count, void *buffer)
{

    return 0;

}

void tty_init()
{

    ttyDevice.base.module.type = MODULES_TYPE_DEVICE;
    ttyDevice.base.type = TTY_DEVICE_TYPE;
    ttyDevice.cursorOffset = 0;
    ttyDevice.kbdDevice = (struct kbd_device *)modules_get_device(KBD_DEVICE_TYPE);
    ttyDevice.vgaDevice = (struct vga_device *)modules_get_device(VGA_DEVICE_TYPE);
    ttyDevice.vgaDevice->set_cursor_color(TTY_COLOR_WHITE, TTY_COLOR_BLACK);
    tty_clear();

    stream_init(&ttyDevice.in, "stdin", tty_device_node_read, 0);
    stream_init(&ttyDevice.out, "stdout", 0, tty_device_node_write);
    stream_init(&ttyDevice.error, "stderr", 0, tty_device_node_write);
    stream_init(&ttyDevice.cwd, "cwd", tty_cwd_device_node_read, tty_cwd_device_node_write);

    modules_register_device(&ttyDevice.in.base);
    modules_register_device(&ttyDevice.out.base);
    modules_register_device(&ttyDevice.error.base);
    modules_register_device(&ttyDevice.cwd.base);
    modules_register_device(&ttyDevice.base);

}

