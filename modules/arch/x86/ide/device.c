#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include "ide.h"

static void configure_ata(struct ide_device *self)
{

    unsigned int i;
    unsigned short buffer[256];
    unsigned int lba48;
    char *model;

    ide_bus_read_blocks(self->bus, 1, buffer);

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

    unsigned short buffer[256];

    ide_bus_set_command(self->bus, IDE_CONTROL_ID_ATAPI);
    ide_bus_read_blocks(self->bus, 1, buffer);

}

unsigned int ide_device_read_lba28(struct ide_device *device, unsigned int sector, unsigned int count, void *buffer)
{

    ide_bus_select(device->bus, 0xE0 | ((sector >> 24) & 0x0F), device->slave);
    ide_bus_set_lba(device->bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    ide_bus_set_command(device->bus, IDE_CONTROL_PIO28_READ);

    return ide_bus_read_blocks(device->bus, count, buffer) * 512;

}

void ide_device_read_lba28_async(struct ide_device *device, unsigned int sector, unsigned int count, void *buffer)
{

    ide_bus_select(device->bus, 0xE0 | ((sector >> 24) & 0x0F), device->slave);
    ide_bus_set_lba(device->bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    ide_bus_set_command(device->bus, IDE_CONTROL_PIO28_READ);
    ide_bus_sleep(device->bus);

}

unsigned int ide_device_write_lba28(struct ide_device *device, unsigned int sector, unsigned int count, void *buffer)
{

    ide_bus_select(device->bus, 0xE0 | ((sector >> 24) & 0x0F), device->slave);
    ide_bus_set_lba(device->bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    ide_bus_set_command(device->bus, IDE_CONTROL_PIO28_WRITE);

    return ide_bus_write_blocks(device->bus, count, buffer) * 512;

}

void ide_device_write_lba28_async(struct ide_device *device, unsigned int sector, unsigned int count, void *buffer)
{

    ide_bus_select(device->bus, 0xE0 | ((sector >> 24) & 0x0F), device->slave);
    ide_bus_set_lba(device->bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    ide_bus_set_command(device->bus, IDE_CONTROL_PIO28_WRITE);
    ide_bus_sleep(device->bus);

}

unsigned int ide_device_read_lba48(struct ide_device *device, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    ide_bus_select(device->bus, 0x40, device->slave);
    ide_bus_set_lba2(device->bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    ide_bus_set_lba(device->bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    ide_bus_set_command(device->bus, IDE_CONTROL_PIO48_READ);

    return ide_bus_read_blocks(device->bus, count, buffer) * 512;

}

void ide_device_read_lba48_async(struct ide_device *device, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    ide_bus_select(device->bus, 0x40, device->slave);
    ide_bus_set_lba2(device->bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    ide_bus_set_lba(device->bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    ide_bus_set_command(device->bus, IDE_CONTROL_PIO48_READ);
    ide_bus_sleep(device->bus);

}

unsigned int ide_device_write_lba48(struct ide_device *device, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    ide_bus_select(device->bus, 0x40, device->slave);
    ide_bus_set_lba2(device->bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    ide_bus_set_lba(device->bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    ide_bus_set_command(device->bus, IDE_CONTROL_PIO48_WRITE);

    return ide_bus_write_blocks(device->bus, count, buffer) * 512;

}

void ide_device_write_lba48_async(struct ide_device *device, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    ide_bus_select(device->bus, 0x40, device->slave);
    ide_bus_set_lba2(device->bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    ide_bus_set_lba(device->bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    ide_bus_set_command(device->bus, IDE_CONTROL_PIO48_WRITE);
    ide_bus_sleep(device->bus);

}

void ide_init_device(struct ide_device *device, struct ide_bus *bus, unsigned int irq, unsigned int slave, unsigned int type)
{

    memory_clear(device, sizeof (struct ide_device));
    base_init_device(&device->base, IDE_DEVICE_TYPE, irq, "ide");

    device->bus = bus;
    device->type = type;
    device->slave = slave;
    device->configure_ata = configure_ata;
    device->configure_atapi = configure_atapi;

}

