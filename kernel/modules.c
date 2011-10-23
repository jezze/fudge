#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/elf/elf.h>
#include <modules/stream/stream.h>
#include <modules/tty/tty.h>
#include <modules/io/io.h>
#include <modules/ata/ata.h>
#include <modules/kbd/kbd.h>
#include <modules/pci/pci.h>
#include <modules/pit/pit.h>
#include <modules/rtc/rtc.h>
#include <modules/rtl8139/rtl8139.h>
#include <modules/serial/serial.h>
#include <modules/vga/vga.h>

static struct modules_bus *modulesBusses[32];
static struct modules_device *modulesDevices[64];
static struct modules_driver *modulesDrivers[32];
static struct vfs_filesystem modulesFilesystem;
static struct vfs_node modulesRoot;

struct modules_bus *modules_get_bus(unsigned int type)
{

    unsigned int i;

    for (i = 0; i < 32; i++)
    {

        if (!modulesBusses[i])
            continue;

        if (modulesBusses[i]->type == type)
            return modulesBusses[i];

    }

    return 0;

}

struct modules_device *modules_get_device(unsigned int type)
{

    unsigned int i;

    for (i = 0; i < 32; i++)
    {

        if (!modulesDevices[i])
            continue;

        if (modulesDevices[i]->type == type)
            return modulesDevices[i];

    }

    return 0;

}

struct modules_driver *modules_get_driver(unsigned int type)
{

    unsigned int i;

    for (i = 0; i < 32; i++)
    {

        if (!modulesDrivers[i])
            continue;

        if (modulesDrivers[i]->type == type)
            return modulesDrivers[i];

    }

    return 0;

}

void modules_register_bus(struct modules_bus *bus)
{

    unsigned int i;

    for (i = 0; i < 32; i++)
    {

        if (modulesBusses[i])
            continue;

        modulesBusses[i] = bus;

        break;

    }

}

void modules_register_device(struct modules_device *device)
{

    unsigned int i;

    for (i = 0; i < 32; i++)
    {

        if (modulesDevices[i])
            continue;

        modulesDevices[i] = device;

        break;

    }

}

void modules_register_driver(struct modules_driver *driver)
{

    unsigned int i;

    for (i = 0; i < 32; i++)
    {

        if (modulesDrivers[i])
            continue;

        modulesDrivers[i] = driver;

        break;

    }

}

static struct vfs_node *modules_filesystem_lookup(struct vfs_filesystem *self, char *path)
{

    unsigned int i;

    for (i = 0; modulesDevices[i]; i++)
    {

        if (modulesDevices[i]->type != 4000)
            continue;

        unsigned int count = string_length(modulesDevices[i]->name) + 1;

        if (!memory_compare(path, modulesDevices[i]->name, count))
            return &modulesDevices[i]->node;

    }

    return 0;

}

static unsigned int modules_filesystem_node_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    memory_set(buffer, 0, 1);
    unsigned int i;

    for (i = 0; modulesDevices[i]; i++)
    {

        if (modulesDevices[i]->type != 4000)
            continue;

        string_concat(buffer, modulesDevices[i]->name);
        string_concat(buffer, "\n");

    }

    return string_length(buffer);

}

static void modules_init_devices()
{

    io_init();
    vga_init();
    pit_init();
    kbd_init();
    rtc_init();
    pci_init();
    ata_init();
    serial_init();
    tty_init();
    elf_init();
    rtl8139_init();

}

void modules_init()
{

    string_copy(modulesFilesystem.name, "dev");
    modulesFilesystem.root = &modulesRoot;
    modulesFilesystem.root->operations.read = modules_filesystem_node_read;
    modulesFilesystem.lookup = modules_filesystem_lookup;
    vfs_register_filesystem(&modulesFilesystem);

    modules_init_devices();

}

