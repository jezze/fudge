#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/io/io.h>

static struct io_device ioDevice;

void io_init()
{

    string_copy(ioDevice.base.name, "io");
    ioDevice.base.module.type = MODULES_TYPE_DEVICE;
    ioDevice.base.type = IO_DEVICE_TYPE;
    ioDevice.inb = io_inb;
    ioDevice.inw = io_inw;
    ioDevice.ind = io_ind;
    ioDevice.outb = io_outb;
    ioDevice.outw = io_outw;
    ioDevice.outd = io_outd;

    modules_register_device(&ioDevice.base);

}

