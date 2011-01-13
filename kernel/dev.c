#include <lib/cbuffer.h>
#include <lib/call.h>
#include <lib/file.h>
#include <lib/vfs.h>
#include <kernel/vfs.h>
#include <kernel/dev.h>
#include <arch/x86/kernel/io.h>
#include <arch/x86/modules/pit/pit.h>
#include <arch/x86/modules/rtc/rtc.h>
#include <arch/x86/modules/ata/ata.h>
#include <arch/x86/modules/kbd/kbd.h>
#include <arch/x86/modules/vga/vga.h>
#include <modules/tty/tty.h>

struct vfs_node *devEntries[32];

static struct vfs_node *dev_node_walk(struct vfs_node *node, unsigned int index)
{

    if (index < node->length)
        return devEntries[index];
    else
        return 0;

}

static unsigned int dev_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    devEntries[offset] = (struct vfs_node *)buffer;
    node->length++;

    return count;

}

static void dev_init_devices()
{

    io_init();
    vga_init();
    pit_init();
    kbd_init();
    rtc_init();
    ata_init();
    tty_init();

}

void dev_init()
{

    struct vfs_node *node = vfs_add_node("dev", 0);
    node->walk = dev_node_walk;
    node->write = dev_node_write;

    struct vfs_node *root = call_open("/");
    file_write(root, root->length, 1, node);

    dev_init_devices();

}

