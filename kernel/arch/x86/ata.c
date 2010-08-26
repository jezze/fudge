#include <lib/call.h>
#include <kernel/arch/x86/ata.h>
#include <kernel/arch/x86/io.h>

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

unsigned char ata_identify_pata(struct ata_device *device)
{

    while (1)
    {

        unsigned char status = ata_get_command(device);

        if (status & ATA_STATUS_ERROR)
            return 0;
            
        if (!(status & 0x80) && (status & 0x08))
            break;

    }

    device->type = ATA_DEVICE_TYPE_PATA;

    unsigned short buffer[256];

    unsigned int i;

    for (i = 0; i < 256; i++)
        buffer[i] = io_inw(device->data);

    return 1;

}

unsigned char ata_identify_other(struct ata_device *device)
{

    unsigned short lba = io_inb(device->data + ATA_DATA_LBA1) | (io_inb(device->data + ATA_DATA_LBA2) << 8);

    if (lba == 0xEB14 || lba == 0x9669)
    {

        device->type = ATA_DEVICE_TYPE_ATAPI;

        return 1;

    }

    if (lba == 0xC33C)
    {

        device->type = ATA_DEVICE_TYPE_SATA;

        return 1;

    }

    return 0;

}

unsigned char ata_identify(struct ata_device *device)
{

    ata_select(device);
    ata_set_command(device, ATA_COMMAND_IDENTIFY);

    unsigned char status = ata_get_command(device);

    if (!status)
        return 0;

    if (status & ATA_STATUS_ERROR)
        return ata_identify_other(device);
    else
        return ata_identify_pata(device);

}

void ata_print_info(struct ata_device *device)
{

    switch (device->type)
    {

        case ATA_DEVICE_TYPE_PATA:
            call_puts("parallell ata\n");
            break;

        case ATA_DEVICE_TYPE_SATA:
            call_puts("serial ata\n");
            break;

        case ATA_DEVICE_TYPE_ATAPI:
            call_puts("atapi\n");
            break;

        default:
            call_puts("unknown\n");
            break;

    }

}

void ata_init()
{

    call_puts("CHECKING ATA\n");

    primaryMaster.control = ATA_PRIMARY_MASTER_CONTROL;
    primaryMaster.data = ATA_PRIMARY_MASTER_DATA;

    if (ata_identify(&primaryMaster))
    {

        call_puts("hda: ");
        ata_print_info(&primaryMaster);        

    }

    primarySlave.control = ATA_PRIMARY_SLAVE_CONTROL;
    primarySlave.data = ATA_PRIMARY_SLAVE_DATA;

    if (ata_identify(&primarySlave))
    {

        call_puts("hdb: ");
        ata_print_info(&primarySlave);        

    }

    secondaryMaster.control = ATA_SECONDARY_MASTER_CONTROL;
    secondaryMaster.data = ATA_SECONDARY_MASTER_DATA;

    if (ata_identify(&secondaryMaster))
    {

        call_puts("hdc: ");
        ata_print_info(&secondaryMaster);        

    }

    secondarySlave.control = ATA_SECONDARY_SLAVE_CONTROL;
    secondarySlave.data = ATA_SECONDARY_SLAVE_DATA;

    if (ata_identify(&secondarySlave))
    {

        call_puts("hdd: ");
        ata_print_info(&secondarySlave);        

    }

    call_puts("\n");

}

