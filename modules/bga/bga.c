#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <modules/bga/bga.h>

static struct bga_device device;

void bga_device_init(struct bga_device *device)
{

    modules_device_init(&device->base, BGA_DEVICE_TYPE);

}

void init()
{

    bga_device_init(&device);

    modules_register_device(&device.base);

}

void destroy()
{

    modules_unregister_device(&device.base);

}

