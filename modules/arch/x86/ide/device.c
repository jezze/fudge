#include <fudge/memory.h>
#include <system/system.h>
#include <base/base.h>
#include "ide.h"

static void configure_ata(struct ide_device *self)
{

    unsigned int i;
    unsigned short buffer[256];
    unsigned int lba48;
    char *model;

    self->bus->read_blocks(self->bus, 1, buffer);

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

    self->bus->set_command(self->bus, IDE_COMMAND_ID_ATAPI);
    self->bus->read_blocks(self->bus, 1, buffer);

}

static unsigned int read_lba28(struct ide_device *self, unsigned int sector, unsigned int count, void *buffer)
{

    self->bus->select(self->bus, 0xE0 | ((sector >> 24) & 0x0F), self->slave);
    self->bus->set_lba(self->bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    self->bus->set_command(self->bus, IDE_COMMAND_PIO28_READ);

    return self->bus->read_blocks(self->bus, count, buffer) * 512;

}

static void read_lba28_async(struct ide_device *self, unsigned int sector, unsigned int count, void *buffer)
{

    self->bus->select(self->bus, 0xE0 | ((sector >> 24) & 0x0F), self->slave);
    self->bus->set_lba(self->bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    self->bus->set_command(self->bus, IDE_COMMAND_PIO28_READ);
    self->bus->sleep(self->bus);

}

static unsigned int write_lba28(struct ide_device *self, unsigned int sector, unsigned int count, void *buffer)
{

    self->bus->select(self->bus, 0xE0 | ((sector >> 24) & 0x0F), self->slave);
    self->bus->set_lba(self->bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    self->bus->set_command(self->bus, IDE_COMMAND_PIO28_WRITE);

    return self->bus->write_blocks(self->bus, count, buffer) * 512;

}

static void write_lba28_async(struct ide_device *self, unsigned int sector, unsigned int count, void *buffer)
{

    self->bus->select(self->bus, 0xE0 | ((sector >> 24) & 0x0F), self->slave);
    self->bus->set_lba(self->bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    self->bus->set_command(self->bus, IDE_COMMAND_PIO28_WRITE);
    self->bus->sleep(self->bus);

}

static unsigned int read_lba48(struct ide_device *self, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    self->bus->select(self->bus, 0x40, self->slave);
    self->bus->set_lba2(self->bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    self->bus->set_lba(self->bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    self->bus->set_command(self->bus, IDE_COMMAND_PIO48_READ);

    return self->bus->read_blocks(self->bus, count, buffer) * 512;

}

static void read_lba48_async(struct ide_device *self, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    self->bus->select(self->bus, 0x40, self->slave);
    self->bus->set_lba2(self->bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    self->bus->set_lba(self->bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    self->bus->set_command(self->bus, IDE_COMMAND_PIO48_READ);
    self->bus->sleep(self->bus);

}

static unsigned int write_lba48(struct ide_device *self, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    self->bus->select(self->bus, 0x40, self->slave);
    self->bus->set_lba2(self->bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    self->bus->set_lba(self->bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    self->bus->set_command(self->bus, IDE_COMMAND_PIO48_WRITE);

    return self->bus->write_blocks(self->bus, count, buffer) * 512;

}

static void write_lba48_async(struct ide_device *self, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    self->bus->select(self->bus, 0x40, self->slave);
    self->bus->set_lba2(self->bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    self->bus->set_lba(self->bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    self->bus->set_command(self->bus, IDE_COMMAND_PIO48_WRITE);
    self->bus->sleep(self->bus);

}

void ide_init_device(struct ide_device *device, struct ide_bus *bus, unsigned int irq, unsigned int slave, unsigned int type)
{

    memory_clear(device, sizeof (struct ide_device));

    base_init_device(&device->base, IDE_DEVICE_TYPE, "ide");

    device->bus = bus;
    device->type = type;
    device->irq = irq;
    device->slave = slave;
    device->configure_ata = configure_ata;
    device->configure_atapi = configure_atapi;
    device->read_lba28 = read_lba28;
    device->read_lba28_async = read_lba28_async;
    device->write_lba28 = write_lba28;
    device->write_lba28_async = write_lba28_async;
    device->read_lba48 = read_lba48;
    device->read_lba48_async = read_lba48_async;
    device->write_lba48 = write_lba48;
    device->write_lba48_async = write_lba48_async;

}

