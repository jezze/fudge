#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>
#include <modules/ata/ata.h>

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

unsigned int ata_device_check_module(struct modules_module *module)
{

    if (!modules_is_device(module))
        return 0;

    struct modules_device *device = (struct modules_device *)module;

    return device->type == ATA_DEVICE_TYPE;

}

void ata_device_init(struct ata_device *device, struct ata_bus *bus, unsigned int secondary, unsigned int type)
{

    modules_device_init(&device->base, ATA_DEVICE_TYPE);
    device->bus = bus;
    device->pciDevice = 0;
    device->type = type;
    device->secondary = secondary;
    device->lba28Max = 0;
    device->read_lba28 = ata_device_read_lba28;
    device->write_lba28 = ata_device_write_lba28;
    device->lba48MaxLow = 0;
    device->lba48MaxHigh = 0;
    device->read_lba48 = ata_device_read_lba48;
    device->write_lba48 = ata_device_write_lba48;

}

