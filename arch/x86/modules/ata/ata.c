#include <lib/file.h>
#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <arch/x86/modules/io/io.h>
#include <arch/x86/modules/ata/ata.h>

static struct modules_bus ataBus;
static struct ata_device ataPrimaryMaster;
static struct ata_device ataPrimarySlave;
static struct ata_device ataSecondaryMaster;
static struct ata_device ataSecondarySlave;

static void ata_sleep(struct ata_device *device)
{

    io_inb(device->control + ATA_CONTROL_STATUS);
    io_inb(device->control + ATA_CONTROL_STATUS);
    io_inb(device->control + ATA_CONTROL_STATUS);
    io_inb(device->control + ATA_CONTROL_STATUS);

}

static void ata_select(struct ata_device *device)
{

    io_outb(device->data + ATA_DATA_SELECT, device->secondary ? 0xB0 : 0xA0);
    ata_sleep(device);

}

static unsigned char ata_get_command(struct ata_device *device)
{

    return io_inb(device->data + ATA_DATA_COMMAND);

}

static void ata_set_command(struct ata_device *device, unsigned char command)
{

    io_outb(device->data + ATA_DATA_COMMAND, command);
    ata_sleep(device);

}

static unsigned char ata_read_identity(struct ata_device *device)
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

static unsigned char ata_identify(struct ata_device *device)
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

    string_copy(ataBus.name, "ata");
    modules_register_bus(MODULES_BUS_TYPE_ATA, &ataBus);

    ataPrimaryMaster.control = ATA_PRIMARY_MASTER_CONTROL;
    ataPrimaryMaster.data = ATA_PRIMARY_MASTER_DATA;

    if (ata_identify(&ataPrimaryMaster))
        modules_register_device(MODULES_BUS_TYPE_ATA, &ataPrimaryMaster.base);

    ataPrimarySlave.control = ATA_PRIMARY_SLAVE_CONTROL;
    ataPrimarySlave.data = ATA_PRIMARY_SLAVE_DATA;

    if (ata_identify(&ataPrimarySlave))
        modules_register_device(MODULES_BUS_TYPE_ATA, &ataPrimarySlave.base);

    ataSecondaryMaster.control = ATA_SECONDARY_MASTER_CONTROL;
    ataSecondaryMaster.data = ATA_SECONDARY_MASTER_DATA;

    if (ata_identify(&ataSecondaryMaster))
        modules_register_device(MODULES_BUS_TYPE_ATA, &ataSecondaryMaster.base);

    ataSecondarySlave.control = ATA_SECONDARY_SLAVE_CONTROL;
    ataSecondarySlave.data = ATA_SECONDARY_SLAVE_DATA;

    if (ata_identify(&ataSecondarySlave))
        modules_register_device(MODULES_BUS_TYPE_ATA, &ataSecondarySlave.base);

}

