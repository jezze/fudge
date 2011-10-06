#include <lib/call.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/io/io.h>
#include <modules/vga/vga.h>

static struct vga_device vgaDevice;

static unsigned int vga_framebuffer_device_read(unsigned int offset, unsigned int count, void *buffer)
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

static unsigned int vga_framebuffer_device_write(unsigned int offset, unsigned int count, void *buffer)
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

static void vga_set_cursor_color(unsigned char fg, unsigned char bg)
{

    vgaDevice.cursorColor = (bg << 4) | (fg & 0x0F);

}

static void vga_set_cursor_offset(unsigned short offset)
{

    io_outb(0x3D4, 14);
    io_outb(0x3D5, offset >> 8);
    io_outb(0x3D4, 15);
    io_outb(0x3D5, offset);

}

void vga_init()
{

    vgaDevice.base.module.type = MODULES_TYPE_DEVICE;
    vgaDevice.base.type = MODULES_DEVICE_TYPE_VGA;
    string_copy(vgaDevice.base.name, "vga");
    vgaDevice.read_framebuffer = vga_framebuffer_device_read;
    vgaDevice.write_framebuffer = vga_framebuffer_device_write;
    vgaDevice.set_cursor_color = vga_set_cursor_color;
    vgaDevice.set_cursor_offset = vga_set_cursor_offset;
    modules_register_device(&vgaDevice.base);

}

