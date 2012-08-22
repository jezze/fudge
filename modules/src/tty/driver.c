#include <memory.h>
#include <string.h>
#include <modules.h>
#include <arch/x86/vga/vga.h>
#include <tty/tty.h>

static void clear(struct tty_driver *self)
{

    char c = ' ';
    int i;

    for (i = 0; i < TTY_CHARACTER_SIZE; i++)
        vga_write_framebuffer(i, 1, &c);

}

static void scroll(struct tty_driver *self)
{

    unsigned int i;
    char buffer[TTY_CHARACTER_SIZE];

    vga_read_framebuffer(TTY_CHARACTER_WIDTH, TTY_CHARACTER_SIZE - TTY_CHARACTER_WIDTH, buffer);

    for (i = TTY_CHARACTER_SIZE - TTY_CHARACTER_WIDTH; i < TTY_CHARACTER_SIZE; i++)
        buffer[i] = ' ';

    vga_write_framebuffer(0, TTY_CHARACTER_SIZE, buffer);

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

        vga_write_framebuffer(self->cursorOffset, 1, &c);
        self->cursorOffset++;

    }

    if (self->cursorOffset >= TTY_CHARACTER_WIDTH * TTY_CHARACTER_HEIGHT)
        self->scroll(self);

}

static void start(struct modules_driver *self)
{

    struct tty_driver *driver = (struct tty_driver *)self;

    driver->clear(driver);

}

void tty_driver_init(struct tty_driver *driver, char *cwdname)
{

    memory_clear(driver, sizeof (struct tty_driver));

    modules_driver_init(&driver->base, TTY_DRIVER_TYPE, "tty", start, 0, 0);

    memory_copy(driver->cwdname, cwdname, string_length(cwdname) + 1);
    driver->clear = clear;
    driver->scroll = scroll;
    driver->putc = putc;

}

