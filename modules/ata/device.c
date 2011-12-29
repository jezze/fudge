#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/ata/ata.h>

static void ata_device_configure_ata(struct ata_device *self, unsigned short *buffer)
{

    unsigned int lba48 = buffer[ATA_ID_SUPPORT] & (1 << 10);

    self->lba28Max = (buffer[ATA_ID_LBA28MAX] << 16) | buffer[ATA_ID_LBA28MAX + 1];

    if (lba48)
    {

        self->lba48MaxLow = (buffer[ATA_ID_LBA48MAX + 0] << 16) | buffer[ATA_ID_LBA48MAX + 1];
        self->lba48MaxHigh = (buffer[ATA_ID_LBA48MAX + 2] << 16) | buffer[ATA_ID_LBA48MAX + 3];

    }

    unsigned int i;

    char *model = (char *)&buffer[ATA_ID_MODEL];

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

static void ata_device_configure_atapi(struct ata_device *self, unsigned short *buffer)
{

}

static unsigned int ata_device_read_lba28(struct ata_device *self, unsigned int sector, unsigned int count, void *buffer)
{

    struct ata_bus *bus = self->bus;

    bus->select(bus, 0xE0 | ((sector >> 24) & 0x0F), self->secondary);
    bus->set_lba(bus, (unsigned char)(count), (unsigned char)(sector >> 0), (unsigned char)(sector >> 8), (unsigned char)(sector >> 16));
    bus->set_command(bus, ATA_COMMAND_PIO28_READ);

    return bus->read_blocks(bus, count, buffer) * 512;

}

static unsigned int ata_device_write_lba28(struct ata_device *self, unsigned int sector, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int ata_device_read_lba48(struct ata_device *self, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    struct ata_bus *bus = self->bus;

    bus->select(bus, 0x40, self->secondary);
    bus->set_lba2(bus, (unsigned char)(count >> 8), (unsigned char)(sectorhigh >> 0), (unsigned char)(sectorhigh >> 8), (unsigned char)(sectorhigh >> 16));
    bus->set_lba(bus, (unsigned char)(count), (unsigned char)(sectorlow >> 0), (unsigned char)(sectorlow >> 8), (unsigned char)(sectorlow >> 16));
    bus->set_command(bus, ATA_COMMAND_PIO48_READ);

    return bus->read_blocks(bus, count, buffer) * 512;

}

static unsigned int ata_device_write_lba48(struct ata_device *self, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    return 0;

}

void ata_device_init(struct ata_device *device, struct ata_bus *bus, unsigned int secondary, unsigned int type)
{

    modules_device_init(&device->base, ATA_DEVICE_TYPE);
    device->bus = bus;
    device->type = type;
    device->secondary = secondary;
    device->configure_ata = ata_device_configure_ata;
    device->configure_atapi = ata_device_configure_atapi;
    device->lba28Max = 0;
    device->read_lba28 = ata_device_read_lba28;
    device->write_lba28 = ata_device_write_lba28;
    device->lba48MaxLow = 0;
    device->lba48MaxHigh = 0;
    device->read_lba48 = ata_device_read_lba48;
    device->write_lba48 = ata_device_write_lba48;

}

