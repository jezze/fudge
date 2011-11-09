#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/io/io.h>
#include <modules/ata/ata.h>

static struct ata_bus ataBusPrimary;
static struct ata_bus ataBusSecondary;
static struct ata_device ataDevices[4];

static void ata_device_sleep(struct ata_device *self)
{

    io_inb(self->control + ATA_CONTROL_STATUS);
    io_inb(self->control + ATA_CONTROL_STATUS);
    io_inb(self->control + ATA_CONTROL_STATUS);
    io_inb(self->control + ATA_CONTROL_STATUS);

}

static void ata_device_select(struct ata_device *self)
{

    io_outb(self->data + ATA_DATA_SELECT, self->secondary ? 0xB0 : 0xA0);
    self->sleep(self);

}

static unsigned char ata_device_get_command(struct ata_device *self)
{

    return io_inb(self->data + ATA_DATA_COMMAND);

}

static void ata_device_set_command(struct ata_device *self, unsigned char command)
{

    io_outb(self->data + ATA_DATA_COMMAND, command);
    self->sleep(self);

}

static unsigned char ata_read_identity(struct ata_device *device)
{

    while (1)
    {

        unsigned char status = device->get_command(device);

        if (status & ATA_STATUS_FLAG_ERROR)
            return 0;
            
        if (!(status & ATA_STATUS_FLAG_BUSY) && (status & ATA_STATUS_FLAG_DRQ))
            break;

    }

    unsigned short buffer[256];

    memory_set(buffer, 0, 512);

    unsigned int i;

    for (i = 0; i < 256; i++)
        buffer[i] = io_inw(device->data);

    log_write("[ata] Ctrl num: %x\n", buffer[0]);

    return 1;

}

static unsigned int ata_device_identify(struct ata_device *self)
{

    self->select(self);
    self->set_command(self, ATA_COMMAND_ID);

    unsigned char status = self->get_command(self);

    if (!status)
        return 0;

    unsigned short lba = io_inb(self->data + ATA_DATA_LBA1) | (io_inb(self->data + ATA_DATA_LBA2) << 8);

    if (lba == 0x0000)
    {

        self->type = ATA_DEVICE_TYPE_ATA;
        ata_read_identity(self);

        return 1;

    }

    if (lba == 0xEB14)
    {

        self->type = ATA_DEVICE_TYPE_ATAPI;

        return 1;

    }

    if (lba == 0xC33C)
    {

        self->type = ATA_DEVICE_TYPE_SATA;

        return 1;

    }

    if (lba == 0x9669)
    {

        self->type = ATA_DEVICE_TYPE_SATAPI;

        return 1;

    }

    return 0;

}

void ata_bus_init(struct ata_bus *bus)
{

    modules_bus_init(&bus->base, ATA_BUS_TYPE);

}

void ata_device_init(struct ata_device *device, unsigned int control, unsigned int data)
{

    modules_device_init(&device->base, ATA_DEVICE_TYPE);
    device->control = control;
    device->data = data;
    device->get_command = ata_device_get_command;
    device->identify = ata_device_identify;
    device->select = ata_device_select;
    device->sleep = ata_device_sleep;
    device->set_command = ata_device_set_command;
       
}

void init()
{

    ata_bus_init(&ataBusPrimary);
    modules_register_bus(&ataBusPrimary.base);

    ata_bus_init(&ataBusSecondary);
    modules_register_bus(&ataBusSecondary.base);

    struct ata_device *device;
    
    device = &ataDevices[0];

    ata_device_init(device, ATA_PRIMARY_MASTER_CONTROL, ATA_PRIMARY_MASTER_DATA);

    if (device->identify(device))
        modules_register_device(&device->base);

    device = &ataDevices[1];

    ata_device_init(device, ATA_PRIMARY_SLAVE_CONTROL, ATA_PRIMARY_SLAVE_DATA);

    if (device->identify(device))
        modules_register_device(&device->base);

    device = &ataDevices[2];

    ata_device_init(device, ATA_SECONDARY_MASTER_CONTROL, ATA_SECONDARY_MASTER_DATA);

    if (device->identify(device))
        modules_register_device(&device->base);

    device = &ataDevices[3];

    ata_device_init(device, ATA_SECONDARY_SLAVE_CONTROL, ATA_SECONDARY_SLAVE_DATA);

    if (device->identify(device))
        modules_register_device(&device->base);

}

