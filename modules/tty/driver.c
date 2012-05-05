#include <lib/memory.h>
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
        self->vgaDriver->write_framebuffer(self->vgaDriver, i, 1, &c);

}

static void scroll(struct tty_driver *self)
{

    char buffer[TTY_CHARACTER_SIZE];

    self->vgaDriver->read_framebuffer(self->vgaDriver, TTY_CHARACTER_WIDTH, TTY_CHARACTER_SIZE - TTY_CHARACTER_WIDTH, buffer);

    unsigned int i;

    for (i = TTY_CHARACTER_SIZE - TTY_CHARACTER_WIDTH; i < TTY_CHARACTER_SIZE; i++)
        buffer[i] = ' ';

    self->vgaDriver->write_framebuffer(self->vgaDriver, 0, TTY_CHARACTER_SIZE, buffer);

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

        self->vgaDriver->write_framebuffer(self->vgaDriver, self->cursorOffset, 1, &c);
        self->cursorOffset++;

    }

    if (self->cursorOffset >= TTY_CHARACTER_WIDTH * TTY_CHARACTER_HEIGHT)
        self->scroll(self);

}

static void start(struct modules_driver *self)
{

    struct tty_driver *driver = (struct tty_driver *)self;

    driver->vgaDriver->set_cursor_color(driver->vgaDriver, TTY_COLOR_WHITE, TTY_COLOR_BLACK);
    driver->clear(driver);

}

void tty_driver_init(struct tty_driver *driver, struct ps2_kbd_driver *kbdDriver, struct vga_driver *vgaDriver, char *cwdname)
{

    memory_clear(driver, sizeof (struct tty_driver));

    modules_driver_init(&driver->base, TTY_DRIVER_TYPE, "tty", start, 0, 0);

    memory_copy(driver->cwdname, cwdname, string_length(cwdname) + 1);
    driver->kbdDriver = kbdDriver;
    driver->vgaDriver = vgaDriver;
    driver->clear = clear;
    driver->scroll = scroll;
    driver->putc = putc;

}

