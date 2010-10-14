#include <lib/call.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <arch/x86/kernel/io.h>
#include <arch/x86/kernel/vga.h>

struct vfs_node vgaFbNode;

unsigned int vga_fb_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned int j = 0;

    for (i = offset; i < offset + count; i++, j++)
    {

        unsigned short *position = (unsigned short *)(VGA_FB_ADDRESS + i * 2);
        *position = ((char *)buffer)[j] | ((VGA_FB_COLOR_BLACK << 4) | (VGA_FB_COLOR_WHITE & 0x0F) << 8);

    }

    return count;

}

static void vga_fb_init()
{

    memory_set(&vgaFbNode, 0, sizeof (struct vfs_node));
    string_copy(vgaFbNode.name, "vga");
    vgaFbNode.write = vga_fb_write;

    struct vfs_node *node = call_open("dev");
    vfs_write(node, node->length, 1, &vgaFbNode);

}

void vga_init()
{

    vga_fb_init();

}

