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

static struct modules_bus *modulesBusses[32];
static struct modules_device *modulesDevices[32];
static struct modules_driver *modulesDrivers[32];
static struct vfs_filesystem modulesFilesystem;
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

    unsigned int i;

    for (i = 0; i < 32; i++)
    {

        if (!modulesDevices[i])
        {

            modulesDevices[i] = device;

            return;

        }

    }

}

void modules_register_driver(unsigned int type, struct modules_driver *driver)
{

    unsigned int i;

    for (i = 0; i < 32; i++)
    {

        if (!modulesDrivers[i])
        {

            modulesDrivers[i] = driver;

            return;

        }

    }

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

static unsigned int modules_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    modulesEntries[offset] = (struct vfs_node *)buffer;
    node->length++;

    return count;

}

static struct vfs_node *modules_filesystem_lookup(struct vfs_filesystem *filesystem, char *path)
{

    unsigned int i;

    for (i = 0; modulesEntries[i]; i++)
    {

        unsigned int count = string_length(modulesEntries[i]->name) + 1;

        if (!memory_compare(path, modulesEntries[i]->name, count))
            return modulesEntries[i];

    }

    return 0;

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
    modulesRoot.write = modules_node_write;
    modulesRoot.read = modules_node_read;

    string_copy(modulesFilesystem.name, "sysfs");
    modulesFilesystem.root = &modulesRoot;
    modulesFilesystem.lookup = modules_filesystem_lookup;
    vfs_register_filesystem(&modulesFilesystem);

    modules_init_devices();

}

