#include <lib/call.h>
#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <arch/x86/kernel/io.h>
#include <arch/x86/kernel/vga.h>

struct vfs_node vgaFbNode;
struct vfs_node vgaFbColorNode;
struct vfs_node vgaFbCursorNode;
unsigned char vgaFbColor;

static unsigned int vga_fb_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned int j = 0;

    for (i = offset; i < offset + count; i++, j++)
    {

        if (i == VGA_FB_SIZE)
            return j - 1;

        memory_copy(buffer + j, (void *)(VGA_FB_ADDRESS + i * 2), 1);

    }

    return j;

}

static unsigned int vga_fb_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned int j = 0;

    for (i = offset; i < offset + count; i++, j++)
    {

        if (i == VGA_FB_SIZE)
            return j - 1;

        memory_copy((void *)(VGA_FB_ADDRESS + i * 2), buffer + j, 1);
        memory_set((void *)(VGA_FB_ADDRESS + i * 2 + 1), vgaFbColor, 1);

    }

    return j;

}

static unsigned int vga_fb_color_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    if (count != 1)
        return 0;

    ((char *)buffer)[0] = vgaFbColor;

    return 1;

}

static unsigned int vga_fb_color_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    if (count != 1)
        return 0;

    vgaFbColor = ((char *)buffer)[0];

    return 1;

}

static unsigned int vga_fb_cursor_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    if (count != 1)
        return 0;

    short position = ((short *)buffer)[0];

    io_outb(0x3D4, 14);
    io_outb(0x3D5, position >> 8);
    io_outb(0x3D4, 15);
    io_outb(0x3D5, position);

    return 1;

}

void vga_init()
{

    string_copy(vgaFbNode.name, "vga_fb");
    vgaFbColorNode.length = VGA_FB_SIZE;
    vgaFbNode.length =  2000;
    vgaFbNode.read = vga_fb_read;
    vgaFbNode.write = vga_fb_write;

    string_copy(vgaFbColorNode.name, "vga_fb_color");
    vgaFbColorNode.length = 1;
    vgaFbColorNode.read = vga_fb_color_read;
    vgaFbColorNode.write = vga_fb_color_write;

    string_copy(vgaFbCursorNode.name, "vga_fb_cursor");
    vgaFbCursorNode.length = 1;
    vgaFbCursorNode.write = vga_fb_cursor_write;

    struct vfs_node *node = call_open("/dev");
    file_write(node, node->length, 1, &vgaFbNode);
    file_write(node, node->length, 1, &vgaFbColorNode);
    file_write(node, node->length, 1, &vgaFbCursorNode);

}

