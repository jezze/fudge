#include <lib/call.h>
#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/elf/elf.h>
#include <modules/tty/tty.h>
#include <arch/x86/modules/io/io.h>
#include <arch/x86/modules/ata/ata.h>
#include <arch/x86/modules/kbd/kbd.h>
#include <arch/x86/modules/pit/pit.h>
#include <arch/x86/modules/rtc/rtc.h>
#include <arch/x86/modules/serial/serial.h>
#include <arch/x86/modules/vga/vga.h>

static struct modules_filesystem modulesFilesystem;

static struct modules_bus *modulesBusses[32];
static struct modules_filesystem *modulesFilesystems[8];

static struct vfs_node *modulesEntries[32];
static struct vfs_node modulesRoot;

void modules_register_bus(unsigned int type, struct modules_bus *bus)
{

    unsigned int i;

    for (i = 0; i < 32; i++)
    {

        if (!modulesBusses[i])
        {

            modulesBusses[i] = bus;

            return;

        }

    }

}

void modules_register_device(unsigned int type, struct modules_device *device)
{

}

void modules_register_driver(unsigned int type, struct modules_driver *driver)
{

}

void modules_register_filesystem(struct modules_filesystem *filesystem)
{

    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        if (!modulesFilesystems[i])
        {

            modulesFilesystems[i] = filesystem;

            return;

        }

    }

}

static struct vfs_node *modules_node_walk(struct vfs_node *node, unsigned int index)
{

    return (index < node->length) ? modulesEntries[index] : 0;

}

static unsigned int modules_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    modulesEntries[offset] = (struct vfs_node *)buffer;
    node->length++;

    return count;

}

static unsigned int modules_node_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    memory_set(buffer, 0, 1);
    unsigned int i;

    for (i = 0; i < node->length; i++)
    {

        string_concat(buffer, modulesEntries[i]->name);
        string_concat(buffer, "\n");

    }

    return string_length(buffer);

}

struct vfs_node *modules_filesystem_lookup(struct modules_filesystem *filesystem, unsigned int id)
{

    return modulesEntries[id];

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
    elf_init();

}

void modules_init()
{

    string_copy(modulesRoot.name, "dev");
    modulesRoot.length = 0;
    modulesRoot.walk = modules_node_walk;
    modulesRoot.write = modules_node_write;
    modulesRoot.read = modules_node_read;

    //TODO remove
    struct vfs_node *rootNode = vfs_get_root();
    rootNode->write(rootNode, rootNode->length, 1, &modulesRoot);

    string_copy(modulesFilesystem.name, "sysfs");
    modulesFilesystem.root = &modulesRoot;
    modulesFilesystem.lookup = modules_filesystem_lookup;
    modules_register_filesystem(&modulesFilesystem);

    modules_init_devices();

}

