#include <lib/call.h>
#include <lib/file.h>
#include <lib/vfs.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <arch/x86/modules/io/io.h>
#include <arch/x86/modules/ata/ata.h>
#include <arch/x86/modules/kbd/kbd.h>
#include <arch/x86/modules/pit/pit.h>
#include <arch/x86/modules/rtc/rtc.h>
#include <arch/x86/modules/serial/serial.h>
#include <arch/x86/modules/vga/vga.h>
#include <modules/tty/tty.h>

struct vfs_node *devEntries[32];

static struct vfs_node *modules_node_walk(struct vfs_node *node, unsigned int index)
{

    return (index < node->length) ? devEntries[index] : 0;

}

static unsigned int modules_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    devEntries[offset] = (struct vfs_node *)buffer;
    node->length++;

    return count;

}

static void modules_init_devices()
{

    io_init();
    vga_init();
    pit_init();
    kbd_init();
    rtc_init();
    ata_init();
    serial_init();
    tty_init();

}

void modules_init()
{

    struct vfs_node *devNode = vfs_add_node("dev", 0);
    devNode->walk = modules_node_walk;
    devNode->write = modules_node_write;

    struct vfs_node *root = call_open("/");
    file_write(root, root->length, 1, devNode);

    modules_init_devices();

    struct vfs_node *kbdNode = call_open("/dev/kbd");
    struct vfs_node *ttyNode = call_open("/dev/tty");

    struct vfs_node *stdinNode = vfs_add_node("stdin", kbdNode->length);
    stdinNode->read = kbdNode->read;
    file_write(devNode, devNode->length, 1, stdinNode);

    struct vfs_node *stdoutNode = vfs_add_node("stdout", ttyNode->length);
    stdoutNode->write = ttyNode->write;
    file_write(devNode, devNode->length, 1, stdoutNode);

    struct vfs_node *stderrNode = vfs_add_node("stderr", ttyNode->length);
    stderrNode->write = ttyNode->write;
    file_write(devNode, devNode->length, 1, stderrNode);

}

