#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/ata/ata.h>

static struct ata_bus primary;
static struct ata_bus secondary;

static void sleep(struct ata_device *device)
{

    io_inb(device->control);
    io_inb(device->control);
    io_inb(device->control);
    io_inb(device->control);

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

static unsigned int ata_bus_detect(struct ata_bus *self, unsigned int secondary, void *buffer)
{

    io_outb(self->data + ATA_DATA_SELECT, 0xA0 | secondary);
    io_inb(self->control);
    io_inb(self->control);
    io_inb(self->control);
    io_inb(self->control);

    io_outb(self->data + ATA_DATA_COUNT0, 0);
    io_outb(self->data + ATA_DATA_LBA0, 0);
    io_outb(self->data + ATA_DATA_LBA1, 0);
    io_outb(self->data + ATA_DATA_LBA2, 0);
    io_outb(self->data + ATA_DATA_COMMAND, ATA_COMMAND_ID);

    unsigned char status = io_inb(self->data + ATA_DATA_COMMAND);

    if (!status)
        return 0;

    while (1)
    {

        unsigned char status = io_inb(self->data + ATA_DATA_COMMAND);

        if (!(status & ATA_STATUS_FLAG_BUSY))
            break;

    }

    unsigned short lba = (io_inb(self->data + ATA_DATA_LBA2) << 8) | io_inb(self->data + ATA_DATA_LBA1);

    if (lba == 0xEB14)
        return ATA_DEVICE_TYPE_ATAPI;

    if (lba == 0xC33C)
        return ATA_DEVICE_TYPE_SATA;

    if (lba == 0x9669)
        return ATA_DEVICE_TYPE_SATAPI;

    while (1)
    {

        unsigned char status = io_inb(self->data + ATA_DATA_COMMAND);

        if ((status & ATA_STATUS_FLAG_DRQ))
            break;

        if ((status & ATA_STATUS_FLAG_ERROR))
            return 0;

    }

    unsigned int i;
    unsigned short *out = (unsigned short *)buffer;

    for (i = 0; i < 256; i++)
        *out++ = io_inw(self->data);

    return ATA_DEVICE_TYPE_ATA;

}

void ata_device_init(struct ata_device *device, unsigned int secondary, unsigned int control, unsigned int data, unsigned int type)
{

    modules_device_init(&device->base, ATA_DEVICE_TYPE);
    device->type = type;
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

}

static struct ata_device *ata_bus_find_device(struct ata_bus *self, unsigned int type)
{

    if (self->primary.type == type)
        return &self->primary;

    if (self->secondary.type == type)
        return &self->secondary;

    return 0;

}

static void configure_device(struct ata_device *device, unsigned short *buffer)
{

    if (device->type != ATA_DEVICE_TYPE_ATA)
        return;

    unsigned int lba48 = buffer[ATA_ID_SUPPORT] & (1 << 10);

    device->lba28Max = (buffer[ATA_ID_LBA28MAX] << 16) | buffer[ATA_ID_LBA28MAX + 1];

    if (lba48)
    {

        device->lba48MaxLow = (buffer[ATA_ID_LBA48MAX + 0] << 16) | buffer[ATA_ID_LBA48MAX + 1];
        device->lba48MaxHigh = (buffer[ATA_ID_LBA48MAX + 2] << 16) | buffer[ATA_ID_LBA48MAX + 3];

    }

    unsigned int i;

    char *model = (char *)&buffer[ATA_ID_MODEL];

    for (i = 0; i < 40; i++)
        device->model[i] = model[i + 1 - ((i & 1) << 1)];

    device->model[40] = '\0';

    for (i = 39; i > 0; i--)
    {

        if (device->model[i] == ' ')
            device->model[i] = '\0';
        else
            break;

    }

}

void ata_bus_init(struct ata_bus *bus, unsigned int control, unsigned int data)
{

    modules_bus_init(&bus->base, ATA_BUS_TYPE);
    bus->control = control;
    bus->data = data;
    bus->detect = ata_bus_detect;
    bus->find_device = ata_bus_find_device;

    unsigned short buffer[256];

    unsigned int type;

    if ((type = bus->detect(bus, 0 << 4, buffer)))
    {

        ata_device_init(&bus->primary, 0 << 4, bus->control, bus->data, type);
        configure_device(&bus->primary, buffer);

        modules_register_device(&bus->primary.base);

    }

    if ((type = bus->detect(bus, 1 << 4, buffer)))
    {

        ata_device_init(&bus->secondary, 1 << 4, bus->control, bus->data, type);
        configure_device(&bus->secondary, buffer);

        modules_register_device(&bus->secondary.base);

    }

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

    ata_bus_init(&primary, ATA_MASTER_PRIMARY_CONTROL, ATA_MASTER_PRIMARY_DATA);
    modules_register_bus(&primary.base);

    kernel_register_irq(0x0E, handle_irq_primary);

    ata_bus_init(&secondary, ATA_MASTER_SECONDARY_CONTROL, ATA_MASTER_SECONDARY_DATA);
    modules_register_bus(&secondary.base);

    kernel_register_irq(0x0F, handle_irq_secondary);

}

void destroy()
{

}

