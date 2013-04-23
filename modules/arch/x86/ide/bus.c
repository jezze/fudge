#include <fudge/module.h>
#include <base/base.h>
#include <arch/x86/io/io.h>
#include "ide.h"

static void wait(struct ide_bus *bus)
{

    while (io_inb(bus->data + IDE_DATA_COMMAND) & IDE_STATUS_BUSY);

}

void ide_bus_sleep(struct ide_bus *bus)
{

    io_inb(bus->control);
    io_inb(bus->control);
    io_inb(bus->control);
    io_inb(bus->control);

}

void ide_bus_select(struct ide_bus *bus, unsigned char operation, unsigned int slave)
{

    io_outb(bus->data + IDE_DATA_SELECT, operation | slave << 4);
    ide_bus_sleep(bus);

}

void ide_bus_set_lba(struct ide_bus *bus, unsigned char count, unsigned char lba0, unsigned char lba1, unsigned char lba2)
{

    io_outb(bus->data + IDE_DATA_COUNT0, count);
    io_outb(bus->data + IDE_DATA_LBA0, lba0);
    io_outb(bus->data + IDE_DATA_LBA1, lba1);
    io_outb(bus->data + IDE_DATA_LBA2, lba2);

}

void ide_bus_set_lba2(struct ide_bus *bus, unsigned char count, unsigned char lba3, unsigned char lba4, unsigned char lba5)
{

    io_outb(bus->data + IDE_DATA_COUNT1, count);
    io_outb(bus->data + IDE_DATA_LBA3, lba3);
    io_outb(bus->data + IDE_DATA_LBA4, lba4);
    io_outb(bus->data + IDE_DATA_LBA5, lba5);

}

void ide_bus_set_command(struct ide_bus *bus, unsigned char command)
{

    io_outb(bus->data + IDE_DATA_COMMAND, command);

}

unsigned int ide_bus_read_block(struct ide_bus *bus, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = buffer;

    for (i = 0; i < 256; i++)
        *out++ = io_inw(bus->data);

    return 512;

}

unsigned int ide_bus_read_blocks(struct ide_bus *bus, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = buffer;

    for (i = 0; i < count; i++)
    {

        unsigned int j;

        ide_bus_sleep(bus);
        wait(bus);

        for (j = 0; j < 256; j++)
            *out++ = io_inw(bus->data);

    }

    return i;

}

unsigned int ide_bus_write_block(struct ide_bus *bus, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = buffer;

    for (i = 0; i < 256; i++)
        io_outw(bus->data, *out++);

    return 512;

}

unsigned int ide_bus_write_blocks(struct ide_bus *bus, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = buffer;

    for (i = 0; i < count; i++)
    {

        unsigned int j;

        ide_bus_sleep(bus);
        wait(bus);

        for (j = 0; j < 256; j++)
            io_outw(bus->data, *out++);

    }

    return i;


}

static void add_device(struct ide_bus *bus, unsigned int slave, unsigned int type)
{

    struct ide_device *device = &bus->devices.item[bus->devices.count];
    unsigned int irq = (slave) ? IDE_IRQ_SECONDARY : IDE_IRQ_PRIMARY;

    ide_init_device(device, bus, irq, slave, type);

    if (type == IDE_DEVICE_TYPE_ATA)
        device->configure_ata(device);

    if (type == IDE_DEVICE_TYPE_ATAPI)
        device->configure_atapi(device);

    bus->devices.count++;

}

static unsigned int detect(struct ide_bus *bus, unsigned int slave)
{

    unsigned char status;
    unsigned short lba;

    ide_bus_select(bus, 0xA0, slave);
    ide_bus_set_lba(bus, 0, 0, 0, 0);
    ide_bus_set_command(bus, IDE_CONTROL_ID_ATA);

    status = io_inb(bus->data + IDE_DATA_COMMAND);

    if (!status)
        return 0;

    wait(bus);

    lba = (io_inb(bus->data + IDE_DATA_LBA2) << 8) | io_inb(bus->data + IDE_DATA_LBA1);

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
        add_device(bus, 0, type);

    if ((type = detect(bus, 1)))
        add_device(bus, 1, type);

}

void ide_init_bus(struct ide_bus *bus, unsigned short control, unsigned short data)
{

    memory_clear(bus, sizeof (struct ide_bus));
    base_init_bus(&bus->base, IDE_BUS_TYPE, "ide", scan);

    bus->control = control;
    bus->data = data;

}

