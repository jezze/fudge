#include <io.h>
#include <memory.h>
#include <modules.h>
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

        memory_copy(in + j, (void *)address, 1);

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

        memory_copy((void *)address, in + j, 1);
        memory_copy((void *)(address + 1), &self->cursorColor, 1);

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

void vga_driver_init(struct vga_driver *driver)
{

    memory_clear(driver, sizeof (struct vga_driver));

    modules_driver_init(&driver->base, VGA_DRIVER_TYPE, "vga", 0, 0, 0);

    driver->read_framebuffer = read_framebuffer;
    driver->write_framebuffer = write_framebuffer;
    driver->set_cursor_color = set_cursor_color;
    driver->set_cursor_offset = set_cursor_offset;

}

