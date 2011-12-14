#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/ata/ata.h>

static struct ata_bus busPrimary;
static struct ata_bus busSecondary;
static struct ata_device devices[4];

static void sleep(struct ata_device *device)
{

    io_inb(device->control);
    io_inb(device->control);
    io_inb(device->control);
    io_inb(device->control);
    io_inb(device->control);

}

static unsigned int detect(struct ata_device *device, void *buffer)
{

    io_outb(device->data + ATA_DATA_SELECT, 0xA0 | device->secondary);
    sleep(device);

    io_outb(device->data + ATA_DATA_COUNT0, 0);
    io_outb(device->data + ATA_DATA_LBA0, 0);
    io_outb(device->data + ATA_DATA_LBA1, 0);
    io_outb(device->data + ATA_DATA_LBA2, 0);
    io_outb(device->data + ATA_DATA_COMMAND, ATA_COMMAND_ID);

    unsigned char status = io_inb(device->data + ATA_DATA_COMMAND);

    if (!status)
        return 0;

    while (1)
    {

        unsigned char status = io_inb(device->data + ATA_DATA_COMMAND);

        if (!(status & ATA_STATUS_FLAG_BUSY))
            break;

    }

    unsigned short lba = (io_inb(device->data + ATA_DATA_LBA2) << 8) | io_inb(device->data + ATA_DATA_LBA1);

    if (lba == 0xEB14)
        return ATA_DEVICE_TYPE_ATAPI;

    if (lba == 0xC33C)
        return ATA_DEVICE_TYPE_SATA;

    if (lba == 0x9669)
        return ATA_DEVICE_TYPE_SATAPI;

    while (1)
    {

        unsigned char status = io_inb(device->data + ATA_DATA_COMMAND);

        if ((status & ATA_STATUS_FLAG_DRQ))
            break;

        if ((status & ATA_STATUS_FLAG_ERROR))
            return 0;

    }

    unsigned int i;
    unsigned short *out = (unsigned short *)buffer;

    for (i = 0; i < 256; i++)
        *out++ = io_inw(device->data);

    return ATA_DEVICE_TYPE_ATA;

}

static unsigned int ata_device_read_lba28(struct ata_device *self, unsigned int sector, unsigned int count, void *buffer)
{

    io_outb(self->data + ATA_DATA_SELECT, (0xE0 | self->secondary) | ((sector >> 24) & 0x0F));
    sleep(self);

    io_outb(self->data + ATA_DATA_COUNT0, (unsigned char)(count));
    io_outb(self->data + ATA_DATA_LBA0, (unsigned char)(sector >> 0));
    io_outb(self->data + ATA_DATA_LBA1, (unsigned char)(sector >> 8));
    io_outb(self->data + ATA_DATA_LBA2, (unsigned char)(sector >> 16));
    io_outb(self->data + ATA_DATA_COMMAND, ATA_COMMAND_PIO28_READ);

    unsigned int i;
    unsigned short *out = (unsigned short *)buffer;

    for (i = 0; i < count; i++)
    {

        sleep(self);

        while (1)
        {

            unsigned char status = io_inb(self->data + ATA_DATA_COMMAND);

            if (!(status & ATA_STATUS_FLAG_BUSY))
                break;

        }

        unsigned int i;

        for (i = 0; i < 256; i++)
            *out++ = io_inw(self->data);

    }

    return count * 512;

}

static unsigned int ata_device_write_lba28(struct ata_device *self, unsigned int sector, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int ata_device_read_lba48(struct ata_device *self, unsigned int sector, unsigned int count, void *buffer)
{

    io_outb(self->data + ATA_DATA_SELECT, 0x40 | self->secondary);
    sleep(self);

    io_outb(self->data + ATA_DATA_COUNT0, (unsigned char)(count & 0xF0));
    io_outb(self->data + ATA_DATA_LBA0, (unsigned char)(sector >> 12));
    io_outb(self->data + ATA_DATA_LBA1, (unsigned char)(sector >> 16));
    io_outb(self->data + ATA_DATA_LBA2, (unsigned char)(sector >> 24));
    io_outb(self->data + ATA_DATA_COUNT0, (unsigned char)(count & 0x0F));
    io_outb(self->data + ATA_DATA_LBA0, (unsigned char)(sector >> 0));
    io_outb(self->data + ATA_DATA_LBA1, (unsigned char)(sector >> 4));
    io_outb(self->data + ATA_DATA_LBA2, (unsigned char)(sector >> 8));
    io_outb(self->data + ATA_DATA_COMMAND, ATA_COMMAND_PIO48_READ);

    unsigned int i;
    unsigned short *out = (unsigned short *)buffer;

    for (i = 0; i < count; i++)
    {

        sleep(self);

        while (1)
        {

            unsigned char status = io_inb(self->data + ATA_DATA_COMMAND);

            if (!(status & ATA_STATUS_FLAG_BUSY))
                break;

        }

        unsigned int i;

        for (i = 0; i < 256; i++)
            *out++ = io_inw(self->data);

    }

    return count * 512;

}

static unsigned int ata_device_write_lba48(struct ata_device *self, unsigned int sector, unsigned int count, void *buffer)
{

    return 0;

}

void ata_device_init(struct ata_device *device, unsigned int secondary, unsigned int control, unsigned int data)
{

    modules_device_init(&device->base, ATA_DEVICE_TYPE);
    device->secondary = secondary;
    device->control = control;
    device->data = data;
    device->lba28Max = 0;
    device->read_lba28 = ata_device_read_lba28;
    device->write_lba28 = ata_device_write_lba28;
    device->lba48MaxLow = 0;
    device->lba48MaxHigh = 0;
    device->read_lba48 = ata_device_read_lba48;
    device->write_lba48 = ata_device_write_lba48;

    unsigned short buffer[256];

    device->type = detect(device, buffer);

    log_write("[ata] Type: %d\n", device->type);

    if (device->type == ATA_DEVICE_TYPE_ATA)
    {

        device->lba28Max = (buffer[60] << 16) | buffer[61];

        unsigned int lba48 = buffer[83] & (1 << 10);

        if (lba48)
        {

            device->lba48MaxLow = (buffer[100] << 16) | buffer[101];
            device->lba48MaxHigh = (buffer[102] << 16) | buffer[103];

        }

    }

}

static struct ata_device *ata_bus_find_device(struct ata_bus *self, unsigned int type)
{

    unsigned int i;

    for (i = 0; i < 4; i++)
    {

        if (devices[i].type == type)
            return &devices[i];

    }

    return 0;

}

void ata_bus_init(struct ata_bus *bus)
{

    modules_bus_init(&bus->base, ATA_BUS_TYPE);
    bus->find_device = ata_bus_find_device;

}

static void handle_irq_primary()
{

    log_write("[ata] irq 0x0E\n");

}

static void handle_irq_secondary()
{

    log_write("[ata] irq 0x0F\n");

}

void init()
{

    ata_bus_init(&busPrimary);
    modules_register_bus(&busPrimary.base);

    ata_bus_init(&busSecondary);
    modules_register_bus(&busSecondary.base);

    kernel_register_irq(0x0E, handle_irq_primary);
    kernel_register_irq(0x0F, handle_irq_secondary);

    ata_device_init(&devices[0], 0, ATA_MASTER_PRIMARY_CONTROL, ATA_MASTER_PRIMARY_DATA);

    if (&devices[0].type)
        modules_register_device(&devices[0].base);

    ata_device_init(&devices[1], 1 << 4, ATA_MASTER_PRIMARY_CONTROL, ATA_MASTER_PRIMARY_DATA);

    if (&devices[1].type)
        modules_register_device(&devices[1].base);

    ata_device_init(&devices[2], 0, ATA_MASTER_SECONDARY_CONTROL, ATA_MASTER_SECONDARY_DATA);

    if (&devices[2].type)
        modules_register_device(&devices[2].base);

    ata_device_init(&devices[3], 1 << 4, ATA_MASTER_SECONDARY_CONTROL, ATA_MASTER_SECONDARY_DATA);

    if (&devices[3].type)
        modules_register_device(&devices[3].base);

}

void destroy()
{

}

