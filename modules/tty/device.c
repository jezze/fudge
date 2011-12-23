#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/vga/vga.h>
#include <modules/ps2/ps2.h>
#include <modules/tty/tty.h>

static void clear(struct tty_device *device)
{

    char c = ' ';
    int i;

    for (i = 0; i < TTY_CHARACTER_SIZE; i++)
        device->vgaDevice->write_framebuffer(device->vgaDevice, i, 1, &c);

}

static void scroll(struct tty_device *device)
{

    char buffer[TTY_CHARACTER_SIZE];

    device->vgaDevice->read_framebuffer(device->vgaDevice, TTY_CHARACTER_WIDTH, TTY_CHARACTER_SIZE - TTY_CHARACTER_WIDTH, buffer);

    unsigned int i;

    for (i = TTY_CHARACTER_SIZE - TTY_CHARACTER_WIDTH; i < TTY_CHARACTER_SIZE; i++)
        buffer[i] = ' ';

    device->vgaDevice->write_framebuffer(device->vgaDevice, 0, TTY_CHARACTER_SIZE, buffer);

    device->cursorOffset -= TTY_CHARACTER_WIDTH;

}

static void putc(struct tty_device *device, char c)
{

    if (c == '\b')
    {

        device->cursorOffset--;

    }

    else if (c == '\t')
    {

        device->cursorOffset = (device->cursorOffset + 8) & ~(8 - 1);

    }

    else if (c == '\r')
    {

        device->cursorOffset -= (device->cursorOffset % TTY_CHARACTER_WIDTH);

    }

    else if (c == '\n')
    {

        device->cursorOffset += TTY_CHARACTER_WIDTH - (device->cursorOffset % TTY_CHARACTER_WIDTH);

    }
    
    else if (c >= ' ')
    {

        device->vgaDevice->write_framebuffer(device->vgaDevice, device->cursorOffset, 1, &c);
        device->cursorOffset++;

    }

    if (device->cursorOffset >= TTY_CHARACTER_WIDTH * TTY_CHARACTER_HEIGHT)
        device->scroll(device);

}

void tty_device_init(struct tty_device *device, char *cwdname)
{

    modules_device_init(&device->base, TTY_DEVICE_TYPE);
    device->cursorOffset = 0;
    device->kbdDevice = (struct kbd_device *)modules_get_device(KBD_DEVICE_TYPE, 0);
    device->vgaDevice = (struct vga_device *)modules_get_device(VGA_DEVICE_TYPE, 0);
    device->vgaDevice->set_cursor_color(device->vgaDevice, TTY_COLOR_WHITE, TTY_COLOR_BLACK);
    device->clear = clear;
    device->scroll = scroll;
    device->putc = putc;
    string_write(device->cwdname, cwdname);

    device->clear(device);

    device->base.module.view = tty_view_init(device);

}

