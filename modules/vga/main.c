#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <modules/vga/vga.h>

static struct vga_device device;

void init()
{

    vga_device_init(&device);
    modules_register_device(&device.base);

}

void destroy()
{

    modules_unregister_device(&device.base);

}

