#include <lib/file.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <arch/x86/modules/io/io.h>
#include <arch/x86/modules/ata/ata.h>

struct ata_device primaryMaster;
struct ata_device primarySlave;
struct ata_device secondaryMaster;
struct ata_device secondarySlave;

void ata_sleep(struct ata_device *device)
{

    io_inb(device->control + ATA_CONTROL_STATUS);
    io_inb(device->control + ATA_CONTROL_STATUS);
    io_inb(device->control + ATA_CONTROL_STATUS);
    io_inb(device->control + ATA_CONTROL_STATUS);

}

void ata_select(struct ata_device *device)
{

    io_outb(device->data + ATA_DATA_SELECT, device->secondary ? 0xB0 : 0xA0);
    ata_sleep(device);

}

unsigned char ata_get_command(struct ata_device *device)
{

    return io_inb(device->data + ATA_DATA_COMMAND);

}

void ata_set_command(struct ata_device *device, unsigned char command)
{

    io_outb(device->data + ATA_DATA_COMMAND, command);
    ata_sleep(device);

}

unsigned char ata_read_identity(struct ata_device *device)
{

    while (1)
    {

        unsigned char status = ata_get_command(device);

        if (status & ATA_STATUS_FLAG_ERROR)
            return 0;
            
        if (!(status & ATA_STATUS_FLAG_BUSY) && (status & ATA_STATUS_FLAG_DRQ))
            break;

    }

    unsigned short buffer[256];

    unsigned int i;

    for (i = 0; i < 256; i++)
        buffer[i] = io_inw(device->data);

    return 1;

}

unsigned char ata_identify(struct ata_device *device)
{

    ata_select(device);
    ata_set_command(device, ATA_COMMAND_ID);

    unsigned char status = ata_get_command(device);

    if (!status)
        return 0;

    unsigned short lba = io_inb(device->data + ATA_DATA_LBA1) | (io_inb(device->data + ATA_DATA_LBA2) << 8);

    if (lba == 0x0000)
    {

        device->type = ATA_DEVICE_TYPE_ATA;
        ata_read_identity(device);

        return 1;

    }

    if (lba == 0xEB14)
    {

        device->type = ATA_DEVICE_TYPE_ATAPI;

        return 1;

    }

    if (lba == 0xC33C)
    {

        device->type = ATA_DEVICE_TYPE_SATA;

        return 1;

    }

    if (lba == 0x9669)
    {

        device->type = ATA_DEVICE_TYPE_SATAPI;

        return 1;

    }

    return 0;

}

void ata_init()
{

    primaryMaster.control = ATA_PRIMARY_MASTER_CONTROL;
    primaryMaster.data = ATA_PRIMARY_MASTER_DATA;

    if (ata_identify(&primaryMaster))
        modules_register_device(MODULES_BUS_TYPE_ATA, &primaryMaster.base);

    primarySlave.control = ATA_PRIMARY_SLAVE_CONTROL;
    primarySlave.data = ATA_PRIMARY_SLAVE_DATA;

    if (ata_identify(&primarySlave))
        modules_register_device(MODULES_BUS_TYPE_ATA, &primarySlave.base);

    secondaryMaster.control = ATA_SECONDARY_MASTER_CONTROL;
    secondaryMaster.data = ATA_SECONDARY_MASTER_DATA;

    if (ata_identify(&secondaryMaster))
        modules_register_device(MODULES_BUS_TYPE_ATA, &secondaryMaster.base);

    secondarySlave.control = ATA_SECONDARY_SLAVE_CONTROL;
    secondarySlave.data = ATA_SECONDARY_SLAVE_DATA;

    if (ata_identify(&secondarySlave))
        modules_register_device(MODULES_BUS_TYPE_ATA, &secondarySlave.base);

}

