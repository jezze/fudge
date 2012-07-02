#include <memory.h>
#include <modules.h>
#include <ata/ata.h>
#include <mbr/mbr.h>

static unsigned int read(struct mbr_device *self, unsigned int offset, unsigned int count, void *buffer)
{

    return self->ataDevice->read_lba28(self->ataDevice, self->partition.sectorLba + offset, count, buffer);

}

static unsigned int write(struct mbr_device *self, unsigned int offset, unsigned int count, void *buffer)
{

    return self->ataDevice->write_lba28(self->ataDevice, self->partition.sectorLba + offset, count, buffer);

}

void mbr_device_init(struct mbr_device *device, struct ata_device *ataDevice, void *buffer)
{

    memory_clear(device, sizeof (struct mbr_device));

    modules_device_init(&device->base, MBR_DEVICE_TYPE, "mbr");

    device->ataDevice = ataDevice;
    memory_copy(&device->partition, buffer, sizeof (struct mbr_partition));
    device->read = read;
    device->write = write;

}

