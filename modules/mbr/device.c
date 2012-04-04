#include <lib/memory.h>
#include <kernel/modules.h>
#include <modules/ata/ata.h>
#include <modules/mbr/mbr.h>

void mbr_device_init(struct mbr_device *device, struct ata_device *ataDevice, struct mbr_partition *partition)
{

    memory_clear(device, sizeof (struct mbr_device));

    modules_device_init(&device->base, MBR_DEVICE_TYPE, "hda:0:0:0");

    device->ataDevice = ataDevice;
    device->partition = partition;

}

