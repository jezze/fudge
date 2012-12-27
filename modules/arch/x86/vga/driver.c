#include <fudge/memory.h>
#include <base/base.h>
#include <video/video.h>
#include <arch/x86/io/io.h>
#include "vga.h"

static unsigned int read_framebuffer(struct vga_driver *driver, unsigned int offset, unsigned int count, void *buffer)
{

    volatile char *address = (volatile char *)VGA_FB_ADDRESS;
    unsigned int i;
    unsigned int j = 0;
    char *in = buffer;

    for (i = offset; i < offset + count; i++, j++)
    {

        if (i >= VGA_FB_SIZE)
            return j;

        memory_read(in + j, 1, (void *)(address + i * 2), 1, 0);

    }

    return j;

}

static unsigned int write_framebuffer(struct vga_driver *driver, unsigned int offset, unsigned int count, void *buffer)
{

    volatile char *address = (volatile char *)VGA_FB_ADDRESS;
    unsigned int i;
    unsigned int j = 0;
    char *in = buffer;

    for (i = offset; i < offset + count; i++, j++)
    {

        if (i >= VGA_FB_SIZE)
            return j;

        memory_write((void *)(address + i * 2 + 0), 1, in + j, 1, 0);
        memory_write((void *)(address + i * 2 + 1), 1, &driver->cursor.color, 1, 0);

    }

    return j;

}

static void scroll(struct vga_driver *driver)
{

    unsigned int i;
    char buffer[2000];

    read_framebuffer(driver, 80, 2000 - 80, buffer);

    for (i = 2000 - 80; i < 2000; i++)
        buffer[i] = ' ';

    write_framebuffer(driver, 0, 2000, buffer);

    driver->cursor.offset -= 80;

}

static void putc(struct vga_driver *driver, char c)
{

    if (c == '\b')
        driver->cursor.offset--;

    if (c == '\t')
        driver->cursor.offset = (driver->cursor.offset + 8) & ~(8 - 1);

    if (c == '\r')
        driver->cursor.offset -= (driver->cursor.offset % 80);

    if (c == '\n')
        driver->cursor.offset += 80 - (driver->cursor.offset % 80);

    if (c >= ' ')
    {

        write_framebuffer(driver, driver->cursor.offset, 1, &c);
        driver->cursor.offset++;

    }

    if (driver->cursor.offset >= 80 * 25)
        scroll(driver);

    io_outb(0x3D4, 14);
    io_outb(0x3D5, driver->cursor.offset >> 8);
    io_outb(0x3D4, 15);
    io_outb(0x3D5, driver->cursor.offset);

}

static unsigned int read_data(struct video_interface *driver, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, (void *)VGA_FB_ADDRESS, 4000, offset);

}

static unsigned int write_data(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct vga_driver *driver = (struct vga_driver *)self->driver;
    unsigned int i;

    for (i = 0; i < count; i++)
        putc(driver, ((char *)buffer)[i]);

    return count;

}

static void start(struct base_driver *self)
{

    struct vga_driver *driver = (struct vga_driver *)self;
    unsigned int i;

    driver->cursor.color = (VGA_FB_COLOR_BLACK << 4) | (VGA_FB_COLOR_WHITE & 0x0F);

    for (i = 0; i < 2000; i++)
        write_framebuffer(driver, i, 1, " ");

}

void vga_init_driver(struct vga_driver *driver)
{

    memory_clear(driver, sizeof (struct vga_driver));

    base_init_driver(&driver->base, VGA_DRIVER_TYPE, "vga", start, 0, 0);
    video_init_interface(&driver->interface, &driver->base, 0, read_data, write_data);

    driver->interface.xres = 0;
    driver->interface.yres = 0;
    driver->interface.bpp = 0;

}

