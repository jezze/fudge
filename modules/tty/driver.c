#include <lib/string.h>
#include <kernel/modules.h>
#include <modules/vga/vga.h>
#include <modules/ps2/ps2.h>
#include <modules/tty/tty.h>

static void clear(struct tty_driver *self)
{

    char c = ' ';
    int i;

    for (i = 0; i < TTY_CHARACTER_SIZE; i++)
        self->vgaDevice->write_framebuffer(self->vgaDevice, i, 1, &c);

}

static void scroll(struct tty_driver *self)
{

    char buffer[TTY_CHARACTER_SIZE];

    self->vgaDevice->read_framebuffer(self->vgaDevice, TTY_CHARACTER_WIDTH, TTY_CHARACTER_SIZE - TTY_CHARACTER_WIDTH, buffer);

    unsigned int i;

    for (i = TTY_CHARACTER_SIZE - TTY_CHARACTER_WIDTH; i < TTY_CHARACTER_SIZE; i++)
        buffer[i] = ' ';

    self->vgaDevice->write_framebuffer(self->vgaDevice, 0, TTY_CHARACTER_SIZE, buffer);

    self->cursorOffset -= TTY_CHARACTER_WIDTH;

}

static void putc(struct tty_driver *self, char c)
{

    if (c == '\b')
    {

        self->cursorOffset--;

    }

    else if (c == '\t')
    {

        self->cursorOffset = (self->cursorOffset + 8) & ~(8 - 1);

    }

    else if (c == '\r')
    {

        self->cursorOffset -= (self->cursorOffset % TTY_CHARACTER_WIDTH);

    }

    else if (c == '\n')
    {

        self->cursorOffset += TTY_CHARACTER_WIDTH - (self->cursorOffset % TTY_CHARACTER_WIDTH);

    }
    
    else if (c >= ' ')
    {

        self->vgaDevice->write_framebuffer(self->vgaDevice, self->cursorOffset, 1, &c);
        self->cursorOffset++;

    }

    if (self->cursorOffset >= TTY_CHARACTER_WIDTH * TTY_CHARACTER_HEIGHT)
        self->scroll(self);

}

void tty_driver_init(struct tty_driver *driver, char *cwdname)
{

    modules_driver_init(&driver->base, TTY_DRIVER_TYPE);

    driver->cursorOffset = 0;
    driver->kbdDriver = (struct kbd_driver *)modules_get_driver(KBD_DRIVER_TYPE);
    driver->vgaDevice = (struct vga_device *)modules_get_device(VGA_DEVICE_TYPE);
    driver->vgaDevice->set_cursor_color(driver->vgaDevice, TTY_COLOR_WHITE, TTY_COLOR_BLACK);
    driver->clear = clear;
    driver->scroll = scroll;
    driver->putc = putc;

    string_write(driver->cwdname, cwdname);

    driver->clear(driver);

}

