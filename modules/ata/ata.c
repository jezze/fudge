#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/ata/ata.h>

static struct ata_bus busPrimary;
static struct ata_bus busSecondary;
static struct ata_device devices[4];

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

static unsigned int ata_device_read_lba28(struct ata_device *device, unsigned int sector, unsigned int count, void *buffer)
{

    io_outb(0x1F6, (device->secondary ? 0xF0 : 0xE0) | ((sector >> 24) & 0x0F));
    io_outb(0x1F1, 0x00);
    io_outb(0x1F2, count);
    io_outb(0x1F3, (unsigned char)(sector >> 0));
    io_outb(0x1F4, (unsigned char)(sector >> 8));
    io_outb(0x1F5, (unsigned char)(sector >> 16));
    io_outb(0x1F7, 0x20);

    while (1)
    {

        unsigned char status = device->get_command(device);

        if ((status & (1 << 7)) == 0)
            break;

//        if ((status & (1 << 3)) != 1)
//            continue;

    }

    unsigned short *out = (unsigned short *)buffer;

    unsigned int i;

    for (i = 0; i < 256; i++)
        out[i] = io_inw(0x1F0);

    return 512;

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

    unsigned int i;

    for (i = 0; i < 256; i++)
        buffer[i] = io_inw(device->data);

    log_write("[ata] Ctrl num: 0x%x\n", buffer[0]);

    unsigned int lba28Max = (buffer[60] << 16) | buffer[61];

    log_write("[ata] LBA28 max: 0x%x\n", lba28Max);

    unsigned int lba48Support = buffer[83] & (1 << 10);

    if (lba48Support)
    {

        unsigned int lba48MaxLow = (buffer[100] << 16) | buffer[101];
        unsigned int lba48MaxHigh = (buffer[102] << 16) | buffer[103];

        log_write("[ata] LBA48 max: 0x%x:%x\n", lba48MaxLow, lba48MaxHigh);

    }

    ata_device_read_lba28(device, 0, 0, buffer);

    log_write("[ata] Read: %s\n", (unsigned char *)buffer);

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

    ata_bus_init(&busPrimary);
    modules_register_bus(&busPrimary.base);

    ata_bus_init(&busSecondary);
    modules_register_bus(&busSecondary.base);

    struct ata_device *device;
    
    device = &devices[0];

    ata_device_init(device, ATA_PRIMARY_MASTER_CONTROL, ATA_PRIMARY_MASTER_DATA);

    if (device->identify(device))
        modules_register_device(&device->base);

    device = &devices[1];

    ata_device_init(device, ATA_PRIMARY_SLAVE_CONTROL, ATA_PRIMARY_SLAVE_DATA);

    if (device->identify(device))
        modules_register_device(&device->base);

    device = &devices[2];

    ata_device_init(device, ATA_SECONDARY_MASTER_CONTROL, ATA_SECONDARY_MASTER_DATA);

    if (device->identify(device))
        modules_register_device(&device->base);

    device = &devices[3];

    ata_device_init(device, ATA_SECONDARY_SLAVE_CONTROL, ATA_SECONDARY_SLAVE_DATA);

    if (device->identify(device))
        modules_register_device(&device->base);

}

