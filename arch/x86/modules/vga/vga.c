#include <lib/call.h>
#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <arch/x86/modules/io/io.h>
#include <arch/x86/modules/vga/vga.h>

struct modules_vga_device vgaDevice;
unsigned char vgaFbColor;

static unsigned int vga_read_framebuffer(char *buffer, unsigned int count, unsigned int offset)
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

static unsigned int vga_fb_node_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    return vgaDevice.read_framebuffer(buffer, count, offset);

}

static unsigned int vga_write_framebuffer(char *buffer, unsigned int count, unsigned int offset)
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

static unsigned int vga_fb_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    return vgaDevice.write_framebuffer(buffer, count, offset);

}

static unsigned int vga_fb_color_node_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    if (count != 1)
        return 0;

    ((char *)buffer)[0] = vgaFbColor;

    return 1;

}

static unsigned int vga_fb_color_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
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

static unsigned int vga_fb_cursor_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    if (count != 1)
        return 0;

    short position = ((short *)buffer)[0];

    vgaDevice.set_cursor_offset(position);

    return 1;

}

void vga_init()
{

    vgaDevice.read_framebuffer = vga_read_framebuffer;
    vgaDevice.write_framebuffer = vga_write_framebuffer;
    vgaDevice.set_cursor_offset = vga_set_cursor_offset;

    struct vfs_node *vgaFbNode = vfs_add_node("vga_fb", VGA_FB_SIZE);
    vgaFbNode->read = vga_fb_node_read;
    vgaFbNode->write = vga_fb_node_write;

    struct vfs_node *vgaFbColorNode = vfs_add_node("vga_fb_color", 1);
    vgaFbColorNode->read = vga_fb_color_node_read;
    vgaFbColorNode->write = vga_fb_color_node_write;

    struct vfs_node *vgaFbCursorNode = vfs_add_node("vga_fb_cursor", 1);
    vgaFbCursorNode->write = vga_fb_cursor_node_write;

    struct vfs_node *devNode = call_open("/dev");
    file_write(devNode, devNode->length, 1, vgaFbNode);
    file_write(devNode, devNode->length, 1, vgaFbColorNode);
    file_write(devNode, devNode->length, 1, vgaFbCursorNode);

}

