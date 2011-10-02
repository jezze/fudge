#include <lib/call.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <arch/x86/modules/io/io.h>
#include <arch/x86/modules/vga/vga.h>

static struct modules_device vgaFramebufferDevice;
static struct modules_device vgaFramebufferColorDevice;
static struct modules_device vgaFramebufferCursorDevice;
static unsigned char vgaFbColor;

static unsigned int vga_framebuffer_device_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
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

static unsigned int vga_framebuffer_device_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned int j = 0;

    for (i = offset; i < offset + count; i++, j++)
    {

        if (i == VGA_FB_SIZE)
            return j;

        memory_copy((void *)(VGA_FB_ADDRESS + i * 2), buffer + j, 1);
        memory_set((void *)(VGA_FB_ADDRESS + i * 2 + 1), vgaFbColor, 1);

    }

    return j;

}

static unsigned int vga_framebuffer_color_device_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    if (count != 1)
        return 0;

    ((char *)buffer)[0] = vgaFbColor;

    return 1;

}

static unsigned int vga_framebuffer_color_device_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    if (count != 1)
        return 0;

    vgaFbColor = ((char *)buffer)[0];

    return 1;

}

static void vga_set_cursor_offset(unsigned short offset)
{

    io_outb(0x3D4, 14);
    io_outb(0x3D5, offset >> 8);
    io_outb(0x3D4, 15);
    io_outb(0x3D5, offset);

}

static unsigned int vga_framebuffer_cursor_device_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    if (count != 1)
        return 0;

    short position = ((short *)buffer)[0];

    vga_set_cursor_offset(position);

    return 1;

}

void vga_init()
{

    string_copy(vgaFramebufferDevice.name, "fb");
    vgaFramebufferDevice.node.length = VGA_FB_SIZE;
    vgaFramebufferDevice.node.operations.read = vga_framebuffer_device_read;
    vgaFramebufferDevice.node.operations.write = vga_framebuffer_device_write;
    modules_register_device(MODULES_DEVICE_TYPE_VGA, &vgaFramebufferDevice);

    string_copy(vgaFramebufferColorDevice.name, "color");
    vgaFramebufferColorDevice.node.length = 1;
    vgaFramebufferColorDevice.node.operations.read = vga_framebuffer_color_device_read;
    vgaFramebufferColorDevice.node.operations.write = vga_framebuffer_color_device_write;
    modules_register_device(0x1234, &vgaFramebufferColorDevice);

    string_copy(vgaFramebufferCursorDevice.name, "cursor");
    vgaFramebufferCursorDevice.node.length = 1;
    vgaFramebufferCursorDevice.node.operations.write = vga_framebuffer_cursor_device_write;
    modules_register_device(0x5678, &vgaFramebufferCursorDevice);

}

