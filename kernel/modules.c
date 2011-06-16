#include <lib/call.h>
#include <lib/file.h>
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

static struct modules_bus *busses[32];

static struct modules_module *modules[32];
static struct file_node *devEntries[32];
static unsigned int modulesCount;

void modules_register(unsigned int type, struct modules_module *module)
{

    module->type = type;
    modules[modulesCount] = module;
    modulesCount++;

}

extern void modules_register_bus(unsigned int type, struct modules_bus *bus)
{

    unsigned int i;

    for (i = 0; i < 32; i++)
    {

        if (!busses[i])
        {

            busses[i] = bus;
            return;

        }

    }

}

extern void modules_register_device(unsigned int type, struct modules_device *device)
{

}

extern void modules_register_driver(unsigned int type, struct modules_driver *driver)
{

}

struct modules_module *modules_find(unsigned int type)
{

    unsigned int i;

    for (i = 0; i < modulesCount; i++)
    {

        if (modules[i]->type == type)
            return modules[i];

    }

    return 0;

}

static struct file_node *modules_node_walk(struct file_node *node, unsigned int index)
{

    return (index < node->length) ? devEntries[index] : 0;

}

static unsigned int modules_node_write(struct file_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    devEntries[offset] = (struct file_node *)buffer;
    node->length++;

    return count;

}

static struct file_node *modules_sysnode_walk(struct file_node *node, unsigned int index)
{

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

    struct file_node *devNode = vfs_add_node("dev", 0);
    devNode->walk = modules_node_walk;
    devNode->write = modules_node_write;

    struct file_node *sysNode = vfs_add_node("sys", 0);
    sysNode->walk = modules_sysnode_walk;

    struct file_node *root = call_open("/");
    file_write(root, root->length, 1, devNode);
    file_write(root, root->length, 1, sysNode);

    modules_init_devices();

}

