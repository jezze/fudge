#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include <arch/x86/io/io.h>
#include "ide.h"

static unsigned int read_block(struct ide_bus *self, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = buffer;

    for (i = 0; i < 256; i++)
        *out++ = io_inw(self->data);

    return 512;

}

static unsigned int read_blocks(struct ide_bus *self, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = buffer;

    for (i = 0; i < count; i++)
    {

        unsigned int j;

        self->sleep(self);
        self->wait(self);

        for (j = 0; j < 256; j++)
            *out++ = io_inw(self->data);

    }

    return i;

}

static unsigned int write_block(struct ide_bus *self, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = buffer;

    for (i = 0; i < 256; i++)
        io_outw(self->data, *out++);

    return 512;

}

static unsigned int write_blocks(struct ide_bus *self, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = buffer;

    for (i = 0; i < count; i++)
    {

        unsigned int j;

        self->sleep(self);
        self->wait(self);

        for (j = 0; j < 256; j++)
            io_outw(self->data, *out++);

    }

    return i;


}

static void sleep(struct ide_bus *self)
{

    io_inb(self->control);
    io_inb(self->control);
    io_inb(self->control);
    io_inb(self->control);

}

static void wait(struct ide_bus *self)
{

    while (io_inb(self->data + IDE_DATA_COMMAND) & IDE_STATUS_BUSY);

}

static void select(struct ide_bus *self, unsigned char operation, unsigned int slave)
{

    io_outb(self->data + IDE_DATA_SELECT, operation | slave << 4);
    self->sleep(self);

}

static void set_lba(struct ide_bus *self, unsigned char count, unsigned char lba0, unsigned char lba1, unsigned char lba2)
{

    io_outb(self->data + IDE_DATA_COUNT0, count);
    io_outb(self->data + IDE_DATA_LBA0, lba0);
    io_outb(self->data + IDE_DATA_LBA1, lba1);
    io_outb(self->data + IDE_DATA_LBA2, lba2);

}

static void set_lba2(struct ide_bus *self, unsigned char count, unsigned char lba3, unsigned char lba4, unsigned char lba5)
{

    io_outb(self->data + IDE_DATA_COUNT1, count);
    io_outb(self->data + IDE_DATA_LBA3, lba3);
    io_outb(self->data + IDE_DATA_LBA4, lba4);
    io_outb(self->data + IDE_DATA_LBA5, lba5);

}

static void set_command(struct ide_bus *self, unsigned char command)
{

    io_outb(self->data + IDE_DATA_COMMAND, command);

}

static unsigned int detect(struct ide_bus *self, unsigned int slave)
{

    unsigned char status;
    unsigned short lba;

    self->select(self, 0xA0, slave);
    self->set_lba(self, 0, 0, 0, 0);
    self->set_command(self, IDE_COMMAND_ID_ATA);

    status = io_inb(self->data + IDE_DATA_COMMAND);

    if (!status)
        return 0;

    self->wait(self);

    lba = (io_inb(self->data + IDE_DATA_LBA2) << 8) | io_inb(self->data + IDE_DATA_LBA1);

    if (lba == 0x0000)
        return IDE_DEVICE_TYPE_ATA;

    if (lba == 0xEB14)
        return IDE_DEVICE_TYPE_ATAPI;

    if (lba == 0xC33C)
        return IDE_DEVICE_TYPE_SATA;

    if (lba == 0x9669)
        return IDE_DEVICE_TYPE_SATAPI;

    return 0;

}

static void scan(struct base_bus *self)
{

    unsigned int type;
    struct ide_bus *bus = (struct ide_bus *)self;

    if ((type = detect(bus, 0)))
        bus->add_device(bus, 0, type);

    if ((type = detect(bus, 1)))
        bus->add_device(bus, 1, type);

}

static void add_device(struct ide_bus *self, unsigned int slave, unsigned int type)
{

    struct ide_device *device = &self->devices[self->devicesCount];
    unsigned int irq = (slave) ? IDE_IRQ_SECONDARY : IDE_IRQ_PRIMARY;

    ide_init_device(device, self, irq, slave, type);

    if (type == IDE_DEVICE_TYPE_ATA)
        device->configure_ata(device);

    if (type == IDE_DEVICE_TYPE_ATAPI)
        device->configure_atapi(device);

    base_register_device(&device->base);

    self->devicesCount++;

}

void ide_init_bus(struct ide_bus *bus, unsigned int control, unsigned int data)
{

    memory_clear(bus, sizeof (struct ide_bus));

    base_init_bus(&bus->base, IDE_BUS_TYPE, "ide", scan);

    bus->control = control;
    bus->data = data;
    bus->sleep = sleep;
    bus->wait = wait;
    bus->select = select;
    bus->set_lba = set_lba;
    bus->set_lba2 = set_lba2;
    bus->set_command = set_command;
    bus->read_block = read_block;
    bus->read_blocks = read_blocks;
    bus->write_block = write_block;
    bus->write_blocks = write_blocks;
    bus->add_device = add_device;

}

