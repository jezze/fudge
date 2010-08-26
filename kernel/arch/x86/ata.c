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

unsigned char ata_identify(struct ata_device *device)
{

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

            unsigned short buffer[256];

            unsigned int i;

            for (i = 0; i < 256; i++)
            {

                buffer[i] = io_inw(device->data);

            }

            return 1;

        }

        else
        {

        }

    }

    return 0;

}

void ata_init()
{

    call_puts("CHECKING ATA\n");

    struct ata_device devices[4];

    devices[0].control = ATA_PRIMARY_MASTER_CONTROL;
    devices[0].data = ATA_PRIMARY_MASTER_DATA;
    devices[1].control = ATA_PRIMARY_SLAVE_CONTROL;
    devices[1].data = ATA_PRIMARY_SLAVE_DATA;
    devices[2].control = ATA_SECONDARY_MASTER_CONTROL;
    devices[2].data = ATA_SECONDARY_MASTER_DATA;
    devices[3].control = ATA_SECONDARY_SLAVE_CONTROL;
    devices[3].data = ATA_SECONDARY_SLAVE_DATA;

    if (ata_identify(&devices[0]))
        call_puts("hda found\n");

    if (ata_identify(&devices[1]))
        call_puts("hdb found\n");

    if (ata_identify(&devices[2]))
        call_puts("hdc found\n");

    if (ata_identify(&devices[3]))
        call_puts("hdd found\n");

    call_puts("\n");

}

