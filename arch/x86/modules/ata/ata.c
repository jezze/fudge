#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <arch/x86/modules/io/io.h>
#include <arch/x86/modules/ata/ata.h>

static struct modules_bus ataBusPrimary;
static struct modules_bus ataBusSecondary;
static struct ata_device ataDevices[4];
static unsigned int ataDevicesCount;

static struct ata_device *ata_get_device(struct vfs_node *node)
{

    unsigned int i;

    for (i = 0; i < ataDevicesCount; i++)
    {

        if (&ataDevices[i].base.node == node)
            return &ataDevices[i];

    }

    return 0;

}

static unsigned int ata_write_num(char *out, unsigned int num, unsigned int base)
{

    if (!num)
    {

        out[0] = '0';

        return 1;

    }

    char buffer[32] = {0};

    int i;

    for (i = 30; num && i; --i, num /= base)
        buffer[i] = "0123456789abcdef"[num % base];

    string_copy(out, buffer + i + 1);

    return 32 - i;

}

static unsigned int ata_device_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct ata_device *device = ata_get_device(node);

    if (!device)
        return 0;

    char num[32];

    string_copy(buffer, "Type: ");

    switch (device->type)
    {

        case ATA_DEVICE_TYPE_ATA:

            string_concat(buffer, "ATA\n");

            break;

        case ATA_DEVICE_TYPE_ATAPI:

            string_concat(buffer, "ATAPI\n");

            break;

        case ATA_DEVICE_TYPE_SATA:

            string_concat(buffer, "SATA\n");

            break;

        case ATA_DEVICE_TYPE_SATAPI:

            string_concat(buffer, "SATAPI\n");

            break;

        default:

            string_concat(buffer, "UNKNOWN\n");

            break;

    }

    string_concat(buffer, "Position: ");
    string_concat(buffer, "\n");

    return string_length(buffer);

}

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

void ata_init_busses()
{

    string_copy(ataBusPrimary.name, "ata:0");
    modules_register_bus(MODULES_BUS_TYPE_ATA, &ataBusPrimary);

    string_copy(ataBusSecondary.name, "ata:1");
    modules_register_bus(MODULES_BUS_TYPE_ATA, &ataBusSecondary);

}

void ata_init_devices()
{

    ataDevicesCount = 4;

    string_copy(ataDevices[0].base.name, "ata:0:0");
    ataDevices[0].base.node.length = 0;
    ataDevices[0].base.node.operations.read = ata_device_read;
    ataDevices[0].control = ATA_PRIMARY_MASTER_CONTROL;
    ataDevices[0].data = ATA_PRIMARY_MASTER_DATA;

    if (ata_identify(&ataDevices[0]))
        modules_register_device(MODULES_DEVICE_TYPE_ATA, &ataDevices[0].base);

    string_copy(ataDevices[1].base.name, "ata:0:1");
    ataDevices[1].base.node.length = 0;
    ataDevices[1].base.node.operations.read = ata_device_read;
    ataDevices[1].control = ATA_PRIMARY_SLAVE_CONTROL;
    ataDevices[1].data = ATA_PRIMARY_SLAVE_DATA;

    if (ata_identify(&ataDevices[1]))
        modules_register_device(MODULES_DEVICE_TYPE_ATA, &ataDevices[1].base);

    string_copy(ataDevices[2].base.name, "ata:1:0");
    ataDevices[2].base.node.length = 0;
    ataDevices[2].base.node.operations.read = ata_device_read;
    ataDevices[2].control = ATA_SECONDARY_MASTER_CONTROL;
    ataDevices[2].data = ATA_SECONDARY_MASTER_DATA;

    if (ata_identify(&ataDevices[2]))
        modules_register_device(MODULES_DEVICE_TYPE_ATA, &ataDevices[2].base);

    string_copy(ataDevices[3].base.name, "ata:1:1");
    ataDevices[3].base.node.length = 0;
    ataDevices[3].base.node.operations.read = ata_device_read;
    ataDevices[3].control = ATA_SECONDARY_SLAVE_CONTROL;
    ataDevices[3].data = ATA_SECONDARY_SLAVE_DATA;

    if (ata_identify(&ataDevices[3]))
        modules_register_device(MODULES_DEVICE_TYPE_ATA, &ataDevices[3].base);

}

void ata_init()
{

    ata_init_busses();
    ata_init_devices();

}

