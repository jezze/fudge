#include <lib/string.h>
#include <kernel/modules.h>
#include <modules/io/io.h>

static struct io_device ioDevice;

void io_device_init(struct io_device *device)
{

    device->base.module.type = MODULES_TYPE_DEVICE;
    device->base.type = IO_DEVICE_TYPE;
    device->inb = io_inb;
    device->inw = io_inw;
    device->ind = io_ind;
    device->outb = io_outb;
    device->outw = io_outw;
    device->outd = io_outd;

}

void io_init()
{

    io_device_init(&ioDevice);

    modules_register_device(&ioDevice.base);

}

