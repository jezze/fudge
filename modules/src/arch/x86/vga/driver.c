#include <memory.h>
#include <vfs.h>
#include <base/base.h>
#include <video/video.h>
#include <arch/x86/io/io.h>
#include <arch/x86/vga/vga.h>

static unsigned int read_framebuffer(struct vga_driver *self, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned int j = 0;
    char *in = buffer;

    for (i = offset; i < offset + count; i++, j++)
    {

        volatile char *address;

        if (i >= VGA_FB_SIZE)
            return j;

        address = (volatile char *)(VGA_FB_ADDRESS + i * 2);

        memory_read(in + j, 1, (void *)address, 1, 0);

    }

    return j;

}

static unsigned int write_framebuffer(struct vga_driver *self, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned int j = 0;
    char *in = buffer;

    for (i = offset; i < offset + count; i++, j++)
    {

        volatile char *address;

        if (i >= VGA_FB_SIZE)
            return j;

        address = (volatile char *)(VGA_FB_ADDRESS + i * 2);

        memory_write((void *)(address + 0), 1, in + j, 1, 0);
        memory_write((void *)(address + 1), 1, &self->cursorColor, 1, 0);

    }

    return j;

}

static void set_cursor_color(struct vga_driver *self, unsigned char fg, unsigned char bg)
{

    self->cursorColor = (bg << 4) | (fg & 0x0F);

}

static void set_cursor_offset(struct vga_driver *self, unsigned short offset)
{

    io_outb(0x3D4, 14);
    io_outb(0x3D5, offset >> 8);
    io_outb(0x3D4, 15);
    io_outb(0x3D5, offset);

}

static void clear(struct vga_driver *driver)
{

    unsigned int i;

    for (i = 0; i < 2000; i++)
        driver->write_framebuffer(driver, i, 1, " ");

}

static unsigned int read_data(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, (void *)VGA_FB_ADDRESS, 4000, offset);

}

static void scroll(struct vga_driver *self)
{

    unsigned int i;
    char buffer[2000];

    self->read_framebuffer(self, 80, 2000 - 80, buffer);

    for (i = 2000 - 80; i < 2000; i++)
        buffer[i] = ' ';

    self->write_framebuffer(self, 0, 2000, buffer);

    self->cursorOffset -= 80;

}

static void putc(struct vga_driver *self, char c)
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

        self->cursorOffset -= (self->cursorOffset % 80);

    }

    else if (c == '\n')
    {

        self->cursorOffset += 80 - (self->cursorOffset % 80);

    }

    else if (c >= ' ')
    {

        self->write_framebuffer(self, self->cursorOffset, 1, &c);
        self->cursorOffset++;

    }

    if (self->cursorOffset >= 80 * 25)
        scroll(self);

    self->set_cursor_offset(self, self->cursorOffset);

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

    driver->set_cursor_color(driver, VGA_FB_COLOR_WHITE, VGA_FB_COLOR_BLACK);

    clear(driver);

}

void vga_init_driver(struct vga_driver *driver)
{

    memory_clear(driver, sizeof (struct vga_driver));

    base_init_driver(&driver->base, VGA_DRIVER_TYPE, "vga", start, 0, 0);
    video_init_interface(&driver->interface, &driver->base, 0, read_data, write_data);

    driver->interface.xres = 0;
    driver->interface.yres = 0;
    driver->interface.bpp = 0;

    driver->read_framebuffer = read_framebuffer;
    driver->write_framebuffer = write_framebuffer;
    driver->set_cursor_color = set_cursor_color;
    driver->set_cursor_offset = set_cursor_offset;

}

