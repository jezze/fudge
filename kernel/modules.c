#include <lib/call.h>
#include <lib/file.h>
#include <lib/vfs.h>
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

struct modules_vga_device *modulesVgaDevice;

struct vfs_node *devEntries[32];

unsigned int modules_serial_device_read(struct modules_serial_device *device, char *buffer, unsigned int count)
{

    return device->read(device, buffer, count);

}

unsigned int modules_serial_device_write(struct modules_serial_device *device, char *buffer, unsigned int count)
{

    return device->write(device, buffer, count);

}

unsigned int modules_binary_module_check(struct modules_binary_module *module, void *address)
{

    return module->check(module, address);

}

unsigned int modules_kbd_device_read(struct modules_kbd_device *device, char *buffer)
{

    return device->read(buffer);

}

unsigned int modules_kbd_device_write(struct modules_kbd_device *device, char *buffer)
{

    return device->write(buffer);

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

unsigned int modules_io_device_read(struct modules_io_device *device, char *buffer, unsigned int count, unsigned int offset)
{

    return device->read(buffer, count, offset);

}

unsigned int modules_io_device_write(struct modules_io_device *device, char *buffer, unsigned int count, unsigned int offset)
{

    return device->write(buffer, count, offset);

}

void modules_register_module(struct modules_module *module)
{

    modules[modulesCount] = module;
    modulesCount++;

}

void modules_set_vga_device(struct modules_vga_device *device)
{

    modulesVgaDevice = device;

}

struct modules_vga_device *modules_get_vga_device()
{

    return modulesVgaDevice;

}

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

    modulesCount = 0;

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

