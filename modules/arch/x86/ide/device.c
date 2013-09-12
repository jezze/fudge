#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include "ide.h"

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

static void configure_ata(struct ide_device *self)
{

    struct ide_bus *bus = (struct ide_bus *)self->base.bus;
    unsigned short buffer[256];
    unsigned int lba48;
    char *model;
    unsigned int i;

    ide_bus_read_blocks(bus, 1, buffer);

    lba48 = buffer[IDE_ID_SUPPORT] & (1 << 10);
    self->lba28Max = (buffer[IDE_ID_LBA28MAX] << 16) | buffer[IDE_ID_LBA28MAX + 1];

    if (lba48)
    {

        self->lba48MaxLow = (buffer[IDE_ID_LBA48MAX + 0] << 16) | buffer[IDE_ID_LBA48MAX + 1];
        self->lba48MaxHigh = (buffer[IDE_ID_LBA48MAX + 2] << 16) | buffer[IDE_ID_LBA48MAX + 3];

    }

    model = (char *)&buffer[IDE_ID_MODEL];

    for (i = 0; i < 40; i++)
        self->model[i] = model[i + 1 - ((i & 1) << 1)];

    self->model[40] = '\0';

    for (i = 39; i > 0; i--)
    {

        if (self->model[i] == ' ')
            self->model[i] = '\0';
        else
            break;

    }

}

static void configure_atapi(struct ide_device *self)
{

    struct ide_bus *bus = (struct ide_bus *)self->base.bus;
    unsigned short buffer[256];

    ide_bus_set_command(bus, IDE_CONTROL_IDATAPI);
    ide_bus_read_blocks(bus, 1, buffer);

}

unsigned int ide_device_read_lba28(struct ide_device *device, unsigned int sector, unsigned int count, void *buffer)
{

    struct ide_bus *bus = (struct ide_bus *)device->base.bus;

    ide_bus_select(bus, 0xE0 | ((sector >> 24) & 0x0F), device->slave);
    ide_bus_set_lba(bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    ide_bus_set_command(bus, IDE_CONTROL_PIO28READ);

    return ide_bus_read_blocks(bus, count, buffer) * 512;

}

void ide_device_read_lba28_async(struct ide_device *device, unsigned int sector, unsigned int count, void *buffer)
{

    struct ide_bus *bus = (struct ide_bus *)device->base.bus;

    ide_bus_select(bus, 0xE0 | ((sector >> 24) & 0x0F), device->slave);
    ide_bus_set_lba(bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    ide_bus_set_command(bus, IDE_CONTROL_PIO28READ);
    ide_bus_sleep(bus);

}

unsigned int ide_device_write_lba28(struct ide_device *device, unsigned int sector, unsigned int count, void *buffer)
{

    struct ide_bus *bus = (struct ide_bus *)device->base.bus;

    ide_bus_select(bus, 0xE0 | ((sector >> 24) & 0x0F), device->slave);
    ide_bus_set_lba(bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    ide_bus_set_command(bus, IDE_CONTROL_PIO28WRITE);

    return ide_bus_write_blocks(bus, count, buffer) * 512;

}

void ide_device_write_lba28_async(struct ide_device *device, unsigned int sector, unsigned int count, void *buffer)
{

    struct ide_bus *bus = (struct ide_bus *)device->base.bus;

    ide_bus_select(bus, 0xE0 | ((sector >> 24) & 0x0F), device->slave);
    ide_bus_set_lba(bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    ide_bus_set_command(bus, IDE_CONTROL_PIO28WRITE);
    ide_bus_sleep(bus);

}

unsigned int ide_device_read_lba48(struct ide_device *device, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    struct ide_bus *bus = (struct ide_bus *)device->base.bus;

    ide_bus_select(bus, 0x40, device->slave);
    ide_bus_set_lba2(bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    ide_bus_set_lba(bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    ide_bus_set_command(bus, IDE_CONTROL_PIO48READ);

    return ide_bus_read_blocks(bus, count, buffer) * 512;

}

void ide_device_read_lba48_async(struct ide_device *device, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    struct ide_bus *bus = (struct ide_bus *)device->base.bus;

    ide_bus_select(bus, 0x40, device->slave);
    ide_bus_set_lba2(bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    ide_bus_set_lba(bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    ide_bus_set_command(bus, IDE_CONTROL_PIO48READ);
    ide_bus_sleep(bus);

}

unsigned int ide_device_write_lba48(struct ide_device *device, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    struct ide_bus *bus = (struct ide_bus *)device->base.bus;

    ide_bus_select(bus, 0x40, device->slave);
    ide_bus_set_lba2(bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    ide_bus_set_lba(bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    ide_bus_set_command(bus, IDE_CONTROL_PIO48WRITE);

    return ide_bus_write_blocks(bus, count, buffer) * 512;

}

void ide_device_write_lba48_async(struct ide_device *device, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    struct ide_bus *bus = (struct ide_bus *)device->base.bus;

    ide_bus_select(bus, 0x40, device->slave);
    ide_bus_set_lba2(bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    ide_bus_set_lba(bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    ide_bus_set_command(bus, IDE_CONTROL_PIO48WRITE);
    ide_bus_sleep(bus);

}

void ide_init_device(struct ide_device *device, struct ide_bus *bus, unsigned int irq, unsigned int slave, unsigned int type)
{

    memory_clear(device, sizeof (struct ide_device));
    base_init_device(&device->base, IDE_DEVICE_TYPE, irq, "ide", &bus->base);

    device->type = type;
    device->slave = slave;
    device->configure_ata = configure_ata;
    device->configure_atapi = configure_atapi;

}

