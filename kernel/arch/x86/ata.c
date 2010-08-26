#include <lib/call.h>
#include <kernel/arch/x86/ata.h>
#include <kernel/arch/x86/io.h>

void ata_sleep(struct ata_device *device)
{

    io_inb(device->control + ATA_CONTROL_STATUS);
    io_inb(device->control + ATA_CONTROL_STATUS);
    io_inb(device->control + ATA_CONTROL_STATUS);
    io_inb(device->control + ATA_CONTROL_STATUS);

}

void ata_select(struct ata_device *device, unsigned char secondary)
{

    io_outb(device->data + ATA_DATA_SELECT, secondary ? 0xB0 : 0xA0);
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

void ata_init()
{

    call_puts("CHECK ATA... ");

    struct ata_device primaryMaster;
    primaryMaster.control = ATA_PRIMARY_MASTER_CONTROL;
    primaryMaster.data = ATA_PRIMARY_MASTER_DATA;

    struct ata_device *device = &primaryMaster;

    ata_select(device, 0);
    ata_set_command(device, 0xEC);

    unsigned char status;

    if ((status = ata_get_command(device)))
    {

        while (1)
        {

            status = ata_get_command(device);

            if ((status & 0x01))
                break;

            if (!(status & 0x80) && (status & 0x08))
                break;

        }

        if (!(status & 0x01))
        {

            call_puts("READY TO READ");

            unsigned short buffer[256];

            unsigned int i;

            for (i = 0; i < 256; i++)
            {

                buffer[i] = io_inw(device->data);

            }

        }

        else
        {

            call_puts("ERROR");

        }

    }

}

