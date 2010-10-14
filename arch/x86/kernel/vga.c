#include <lib/call.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/vfs.h>
#include <arch/x86/kernel/io.h>
#include <arch/x86/kernel/vga.h>

struct vfs_node vgaFbNode;
unsigned short vgaFbColor;

void vga_fb_set_color(unsigned char forecolor, unsigned char backcolor)
{

    vgaFbColor = (backcolor << 4) | (forecolor & 0x0F);

}

void vga_fb_clear()
{

    unsigned short blank = ' ' | (vgaFbColor << 8);

    memory_setw((void *)VGA_FB_ADDRESS, blank, 2000);

}

static unsigned int vga_fb_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned int j = 0;

    for (i = offset; i < offset + count; i++, j++)
    {

        if (i == 2000)
            break;

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

        if (i == 2000)
            break;

        memory_copy((void *)(VGA_FB_ADDRESS + i * 2), buffer + j, 1);

    }

    return j;

}

static void vga_fb_init()
{

    vga_fb_set_color(VGA_FB_COLOR_WHITE, VGA_FB_COLOR_BLACK);
    vga_fb_clear();

}

void vga_init()
{

    vga_fb_init();

    memory_set(&vgaFbNode, 0, sizeof (struct vfs_node));
    string_copy(vgaFbNode.name, "vga");
    vgaFbNode.read = vga_fb_read;
    vgaFbNode.write = vga_fb_write;

    struct vfs_node *node = call_open("dev");
    vfs_write(node, node->length, 1, &vgaFbNode);

}

