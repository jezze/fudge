#include <lib/memory.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <modules/vga/vga.h>

static unsigned int read_framebuffer(struct vga_device *self, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned int j = 0;

    for (i = offset; i < offset + count; i++, j++)
    {

        if (i >= VGA_FB_SIZE)
            return j;

        volatile char *address = (char *)(VGA_FB_ADDRESS + i * 2);

        memory_copy(buffer + j, address, 1);

    }

    return j;

}

static unsigned int write_framebuffer(struct vga_device *self, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned int j = 0;

    for (i = offset; i < offset + count; i++, j++)
    {

        if (i >= VGA_FB_SIZE)
            return j;

        volatile char *address = (char *)(VGA_FB_ADDRESS + i * 2);

        memory_copy(address, buffer + j, 1);
        memory_copy(address + 1, &self->cursorColor, 1);

    }

    return j;

}

static void set_cursor_color(struct vga_device *self, unsigned char fg, unsigned char bg)
{

    self->cursorColor = (bg << 4) | (fg & 0x0F);

}

static void set_cursor_offset(struct vga_device *self, unsigned short offset)
{

    io_outb(0x3D4, 14);
    io_outb(0x3D5, offset >> 8);
    io_outb(0x3D4, 15);
    io_outb(0x3D5, offset);

}

void vga_device_init(struct vga_device *device)
{

    modules_device_init(&device->base, VGA_DEVICE_TYPE);

    device->read_framebuffer = read_framebuffer;
    device->write_framebuffer = write_framebuffer;
    device->set_cursor_color = set_cursor_color;
    device->set_cursor_offset = set_cursor_offset;

}

