#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/io/io.h>
#include <modules/vga/vga.h>

static struct vga_device vgaDevice;

static unsigned int vga_device_read_framebuffer(unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned int j = 0;

    for (i = offset; i < offset + count; i++, j++)
    {

        if (i == VGA_FB_SIZE)
            return j;

        memory_copy(buffer + j, (void *)(VGA_FB_ADDRESS + i * 2), 1);

    }

    return j;

}

static unsigned int vga_device_write_framebuffer(unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned int j = 0;

    for (i = offset; i < offset + count; i++, j++)
    {

        if (i == VGA_FB_SIZE)
            return j;

        memory_copy((void *)(VGA_FB_ADDRESS + i * 2), buffer + j, 1);
        memory_set((void *)(VGA_FB_ADDRESS + i * 2 + 1), vgaDevice.cursorColor, 1);

    }

    return j;

}

static void vga_device_set_cursor_color(unsigned char fg, unsigned char bg)
{

    vgaDevice.cursorColor = (bg << 4) | (fg & 0x0F);

}

static void vga_device_set_cursor_offset(unsigned short offset)
{

    io_outb(0x3D4, 14);
    io_outb(0x3D5, offset >> 8);
    io_outb(0x3D4, 15);
    io_outb(0x3D5, offset);

}

void vga_device_init(struct vga_device *device)
{

    device->base.module.type = MODULES_TYPE_DEVICE;
    device->base.type = VGA_DEVICE_TYPE;
    device->read_framebuffer = vga_device_read_framebuffer;
    device->write_framebuffer = vga_device_write_framebuffer;
    device->set_cursor_color = vga_device_set_cursor_color;
    device->set_cursor_offset = vga_device_set_cursor_offset;

}

void vga_init()
{

    vga_device_init(&vgaDevice);

    modules_register_device(&vgaDevice.base);

}

