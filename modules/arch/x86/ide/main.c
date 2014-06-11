#include <module.h>
#include <kernel/resource.h>
#include <base/base.h>
#include <arch/x86/io/io.h>
#include "ide.h"

enum ide_register
{

    IDE_REGISTER_BUS0CONTROL0           = 0x03F6,
    IDE_REGISTER_BUS0CONTROL1           = 0x0376,
    IDE_REGISTER_BUS0DATA0              = 0x01F0,
    IDE_REGISTER_BUS0DATA1              = 0x0170,
    IDE_REGISTER_BUS1CONTROL0           = 0x03E6,
    IDE_REGISTER_BUS1CONTROL1           = 0x0366,
    IDE_REGISTER_BUS1DATA0              = 0x01E8,
    IDE_REGISTER_BUS1DATA1              = 0x0168

};

enum ide_irq
{

    IDE_IRQ_PRIMARY                     = 0x0E,
    IDE_IRQ_SECONDARY                   = 0x0F

};

enum ide_status
{

    IDE_STATUS_ERROR                    = 0x01,
    IDE_STATUS_DRQ                      = 0x08,
    IDE_STATUS_SRV                      = 0x10,
    IDE_STATUS_DF                       = 0x20,
    IDE_STATUS_RDY                      = 0x40,
    IDE_STATUS_BUSY                     = 0x80

};

enum ide_data
{

    IDE_DATA_FEATURE                    = 0x01,
    IDE_DATA_COUNT0                     = 0x02,
    IDE_DATA_LBA0                       = 0x03,
    IDE_DATA_LBA1                       = 0x04,
    IDE_DATA_LBA2                       = 0x05,
    IDE_DATA_SELECT                     = 0x06,
    IDE_DATA_COMMAND                    = 0x07,
    IDE_DATA_COUNT1                     = 0x08,
    IDE_DATA_LBA3                       = 0x09,
    IDE_DATA_LBA4                       = 0x0A,
    IDE_DATA_LBA5                       = 0x0B

};

enum ide_id
{

    IDE_ID_TYPE                         = 0x00,
    IDE_ID_SERIAL                       = 0x0A,
    IDE_ID_MODEL                        = 0x1B,
    IDE_ID_CAP                          = 0x31,
    IDE_ID_VALID                        = 0x35,
    IDE_ID_LBA28MAX                     = 0x3C,
    IDE_ID_SUPPORT                      = 0x53,
    IDE_ID_LBA48MAX                     = 0x64

};

static struct ide_bus primary;
static struct ide_bus secondary;
static struct ide_device devices[2];

static void wait(struct ide_bus *bus)
{

    while (io_inb(bus->data + IDE_DATA_COMMAND) & IDE_STATUS_BUSY);

}

static void ide_bus_sleep(struct ide_bus *bus)
{

    io_inb(bus->control);
    io_inb(bus->control);
    io_inb(bus->control);
    io_inb(bus->control);

}

static void ide_bus_select(struct ide_bus *bus, unsigned char operation, unsigned int slave)
{

    io_outb(bus->data + IDE_DATA_SELECT, operation | slave << 4);
    ide_bus_sleep(bus);

}

static void ide_bus_set_lba(struct ide_bus *bus, unsigned char count, unsigned char lba0, unsigned char lba1, unsigned char lba2)
{

    io_outb(bus->data + IDE_DATA_COUNT0, count);
    io_outb(bus->data + IDE_DATA_LBA0, lba0);
    io_outb(bus->data + IDE_DATA_LBA1, lba1);
    io_outb(bus->data + IDE_DATA_LBA2, lba2);

}

static void ide_bus_set_lba2(struct ide_bus *bus, unsigned char count, unsigned char lba3, unsigned char lba4, unsigned char lba5)
{

    io_outb(bus->data + IDE_DATA_COUNT1, count);
    io_outb(bus->data + IDE_DATA_LBA3, lba3);
    io_outb(bus->data + IDE_DATA_LBA4, lba4);
    io_outb(bus->data + IDE_DATA_LBA5, lba5);

}

static void ide_bus_set_command(struct ide_bus *bus, unsigned char command)
{

    io_outb(bus->data + IDE_DATA_COMMAND, command);

}

/*
static unsigned int ide_bus_read_block(struct ide_bus *bus, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = buffer;

    for (i = 0; i < 256; i++)
        *out++ = io_inw(bus->data);

    return 512;

}
*/

static unsigned int ide_bus_read_blocks(struct ide_bus *bus, unsigned int count, void *buffer)
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

/*
static unsigned int ide_bus_write_block(struct ide_bus *bus, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = buffer;

    for (i = 0; i < 256; i++)
        io_outw(bus->data, *out++);

    return 512;

}
*/

static unsigned int ide_bus_write_blocks(struct ide_bus *bus, unsigned int count, void *buffer)
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

void ide_configure_ata(struct ide_bus *bus)
{

    unsigned short buffer[256];
    unsigned int lba48;
    char *model;
    unsigned int i;

    ide_bus_read_blocks(bus, 1, buffer);

    lba48 = buffer[IDE_ID_SUPPORT] & (1 << 10);
    bus->lba28Max = (buffer[IDE_ID_LBA28MAX] << 16) | buffer[IDE_ID_LBA28MAX + 1];

    if (lba48)
    {

        bus->lba48MaxLow = (buffer[IDE_ID_LBA48MAX + 0] << 16) | buffer[IDE_ID_LBA48MAX + 1];
        bus->lba48MaxHigh = (buffer[IDE_ID_LBA48MAX + 2] << 16) | buffer[IDE_ID_LBA48MAX + 3];

    }

    model = (char *)&buffer[IDE_ID_MODEL];

    for (i = 0; i < 40; i++)
        bus->model[i] = model[i + 1 - ((i & 1) << 1)];

    bus->model[40] = '\0';

    for (i = 39; i > 0; i--)
    {

        if (bus->model[i] == ' ')
            bus->model[i] = '\0';
        else
            break;

    }

}

void ide_configure_atapi(struct ide_bus *bus)
{

    unsigned short buffer[256];

    ide_bus_set_command(bus, IDE_CONTROL_IDATAPI);
    ide_bus_read_blocks(bus, 1, buffer);

}

unsigned int ide_bus_read_lba28(struct ide_bus *bus, unsigned int slave, unsigned int sector, unsigned int count, void *buffer)
{

    ide_bus_select(bus, 0xE0 | ((sector >> 24) & 0x0F), slave);
    ide_bus_set_lba(bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    ide_bus_set_command(bus, IDE_CONTROL_PIO28READ);

    return ide_bus_read_blocks(bus, count, buffer) * 512;

}

void ide_bus_read_lba28_async(struct ide_bus *bus, unsigned int slave, unsigned int sector, unsigned int count, void *buffer)
{

    ide_bus_select(bus, 0xE0 | ((sector >> 24) & 0x0F), slave);
    ide_bus_set_lba(bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    ide_bus_set_command(bus, IDE_CONTROL_PIO28READ);
    ide_bus_sleep(bus);

}

unsigned int ide_bus_write_lba28(struct ide_bus *bus, unsigned int slave, unsigned int sector, unsigned int count, void *buffer)
{

    ide_bus_select(bus, 0xE0 | ((sector >> 24) & 0x0F), slave);
    ide_bus_set_lba(bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    ide_bus_set_command(bus, IDE_CONTROL_PIO28WRITE);

    return ide_bus_write_blocks(bus, count, buffer) * 512;

}

void ide_bus_write_lba28_async(struct ide_bus *bus, unsigned int slave, unsigned int sector, unsigned int count, void *buffer)
{

    ide_bus_select(bus, 0xE0 | ((sector >> 24) & 0x0F), slave);
    ide_bus_set_lba(bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    ide_bus_set_command(bus, IDE_CONTROL_PIO28WRITE);
    ide_bus_sleep(bus);

}

unsigned int ide_bus_read_lba48(struct ide_bus *bus, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    ide_bus_select(bus, 0x40, slave);
    ide_bus_set_lba2(bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    ide_bus_set_lba(bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    ide_bus_set_command(bus, IDE_CONTROL_PIO48READ);

    return ide_bus_read_blocks(bus, count, buffer) * 512;

}

void ide_bus_read_lba48_async(struct ide_bus *bus, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    ide_bus_select(bus, 0x40, slave);
    ide_bus_set_lba2(bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    ide_bus_set_lba(bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    ide_bus_set_command(bus, IDE_CONTROL_PIO48READ);
    ide_bus_sleep(bus);

}

unsigned int ide_bus_write_lba48(struct ide_bus *bus, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    ide_bus_select(bus, 0x40, slave);
    ide_bus_set_lba2(bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    ide_bus_set_lba(bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    ide_bus_set_command(bus, IDE_CONTROL_PIO48WRITE);

    return ide_bus_write_blocks(bus, count, buffer) * 512;

}

void ide_bus_write_lba48_async(struct ide_bus *bus, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    ide_bus_select(bus, 0x40, slave);
    ide_bus_set_lba2(bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    ide_bus_set_lba(bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    ide_bus_set_command(bus, IDE_CONTROL_PIO48WRITE);
    ide_bus_sleep(bus);

}

static void add_device(struct ide_bus *bus, unsigned int slave, unsigned int type)
{

    struct ide_device *device = &devices[slave];

    base_init_device(&device->base, IDE_DEVICE_TYPE, "ide", &bus->base);
    base_register_device(&device->base);

    device->type = type;
    device->slave = slave;

}

static unsigned int detect(struct ide_bus *bus, unsigned int slave)
{

    unsigned char status;
    unsigned short lba;

    ide_bus_select(bus, 0xA0, slave);
    ide_bus_set_lba(bus, 0, 0, 0, 0);
    ide_bus_set_command(bus, IDE_CONTROL_IDATA);

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

static void setup(struct base_bus *self)
{

    unsigned int type;
    struct ide_bus *bus = (struct ide_bus *)self;

    if ((type = detect(bus, 0)))
        add_device(bus, 0, type);

    if ((type = detect(bus, 1)))
        add_device(bus, 1, type);

}

static unsigned short device_irq(struct base_bus *self, unsigned int id)
{

    return (id) ? IDE_IRQ_SECONDARY : IDE_IRQ_PRIMARY;

}

static void init_bus(struct ide_bus *bus, unsigned short control, unsigned short data)
{

    memory_clear(bus, sizeof (struct ide_bus));
    base_init_bus(&bus->base, "ide", setup, device_irq);

    bus->control = control;
    bus->data = data;

}

void init()
{

    init_bus(&primary, IDE_REGISTER_BUS0CONTROL0, IDE_REGISTER_BUS0DATA0);
    base_register_bus(&primary.base);
    init_bus(&secondary, IDE_REGISTER_BUS0CONTROL1, IDE_REGISTER_BUS0DATA1);
    base_register_bus(&secondary.base);

}

void destroy()
{

    base_unregister_bus(&primary.base);
    base_unregister_bus(&secondary.base);

}

