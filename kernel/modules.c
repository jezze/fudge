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

struct modules_module *modules[32];
unsigned int modulesCount;

struct modules_serial_device *modulesSerialDevice;
struct modules_tty_device *modulesTtyDevice;
struct modules_vga_device *modulesVgaDevice;

struct file_node *devEntries[32];

void modules_register(unsigned int type, struct modules_module *module)
{

    module->type = type;
    modules[modulesCount] = module;
    modulesCount++;

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

unsigned int modules_binary_module_check(struct modules_binary_module *module, void *address)
{

    return module->check(module, address);

}

unsigned int modules_tty_device_read(struct modules_tty_device *device, char *buffer, unsigned int count, unsigned int offset)
{

    return device->read(buffer, count, offset);

}

unsigned int modules_tty_device_write(struct modules_tty_device *device, char *buffer, unsigned int count, unsigned int offset)
{

    return device->write(buffer, count, offset);

}

void modules_tty_device_set_color(struct modules_tty_device *device, unsigned char fg, unsigned char bg)
{

    device->set_color(fg, bg);

}

unsigned int modules_vga_device_read_framebuffer(struct modules_vga_device *device, char *buffer, unsigned int count, unsigned int offset)
{

    return device->read_framebuffer(buffer, count, offset);

}

unsigned int modules_vga_device_write_framebuffer(struct modules_vga_device *device, char *buffer, unsigned int count, unsigned int offset)
{

    return device->write_framebuffer(buffer, count, offset);

}

void modules_vga_device_set_cursor_color(struct modules_vga_device *device, unsigned char fg, unsigned char bg)
{

    return device->set_cursor_color(fg, bg);

}

void modules_vga_device_set_cursor_offset(struct modules_vga_device *device, unsigned short offset)
{

    return device->set_cursor_offset(offset);

}

void modules_set_tty_device(struct modules_tty_device *device)
{

    modulesTtyDevice = device;

}

void modules_set_vga_device(struct modules_vga_device *device)
{

    modulesVgaDevice = device;

}

struct modules_tty_device *modules_get_tty_device()
{

    return modulesTtyDevice;

}

struct modules_vga_device *modules_get_vga_device()
{

    return modulesVgaDevice;

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

    struct file_node *root = call_open("/");
    file_write(root, root->length, 1, devNode);

    modules_init_devices();

    struct file_node *kbdNode = call_open("/dev/kbd");
    struct file_node *ttyNode = call_open("/dev/tty");

    struct file_node *stdinNode = vfs_add_node("stdin", kbdNode->length);
    stdinNode->read = kbdNode->read;
    file_write(devNode, devNode->length, 1, stdinNode);

    struct file_node *stdoutNode = vfs_add_node("stdout", ttyNode->length);
    stdoutNode->write = ttyNode->write;
    file_write(devNode, devNode->length, 1, stdoutNode);

    struct file_node *stderrNode = vfs_add_node("stderr", ttyNode->length);
    stderrNode->write = ttyNode->write;
    file_write(devNode, devNode->length, 1, stderrNode);

}

