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

static void sleep(unsigned short control)
{

    io_inb(control + ATA_CONTROL_STATUS);
    io_inb(control + ATA_CONTROL_STATUS);
    io_inb(control + ATA_CONTROL_STATUS);
    io_inb(control + ATA_CONTROL_STATUS);

}

static void select(unsigned short control, unsigned short data, unsigned int secondary)
{

    io_outb(data + ATA_DATA_SELECT, 0xA0 | secondary);
    sleep(control);

}

static unsigned char get_command(unsigned short data)
{

    return io_inb(data + ATA_DATA_COMMAND);

}

static void set_command(unsigned short control, unsigned short data, unsigned char command)
{

    io_outb(data + ATA_DATA_COMMAND, command);
    sleep(control);

}

static unsigned int detect(unsigned short control, unsigned short data, unsigned int secondary)
{

    select(control, data, secondary);
    set_command(control, data, ATA_COMMAND_ID);

    unsigned char status = get_command(data);

    if (!status)
        return 0;

    unsigned short lba = (io_inb(data + ATA_DATA_LBA2) << 8) | io_inb(data + ATA_DATA_LBA1);

    if (lba == 0x0000)
        return ATA_DEVICE_TYPE_ATA;

    if (lba == 0xEB14)
        return ATA_DEVICE_TYPE_ATAPI;

    if (lba == 0xC33C)
        return ATA_DEVICE_TYPE_SATA;

    if (lba == 0x9669)
        return ATA_DEVICE_TYPE_SATAPI;

    return 0;

}

static unsigned int wait(struct ata_device *device)
{

    while (1)
    {

        unsigned char status = get_command(device->data);

        if (status & ATA_STATUS_FLAG_ERROR)
            return 0;
            
        if (!(status & ATA_STATUS_FLAG_BUSY) && (status & ATA_STATUS_FLAG_DRQ))
            break;

    }

    return 1;

}

static unsigned int ata_device_read_lba28(struct ata_device *device, unsigned int sector, unsigned int count, void *buffer)
{

    io_outb(0x1F2, (unsigned char)(count));
    io_outb(0x1F3, (unsigned char)(sector >> 0));
    io_outb(0x1F4, (unsigned char)(sector >> 8));
    io_outb(0x1F5, (unsigned char)(sector >> 16));
    io_outb(0x1F6, (0xE0 | device->secondary) | ((sector >> 24) & 0x0F));
    io_outb(0x1F7, 0x20);

    unsigned int i;
    unsigned short *out = (unsigned short *)buffer;

    for (i = 0; i < count; i++)
    {

        if (!wait(device))
            return 0;

        unsigned int i;

        for (i = 0; i < 256; i++)
            *out++ = io_inw(0x1F0);

    }

    return count * 512;

}

static unsigned int ata_device_write_lba28(struct ata_device *device, unsigned int sector, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int ata_device_read_lba48(struct ata_device *device, unsigned int sector, unsigned int count, void *buffer)
{

    io_outb(0x1F2, (unsigned char)(count & 0xF0));
    io_outb(0x1F3, (unsigned char)(sector >> 12));
    io_outb(0x1F4, (unsigned char)(sector >> 16));
    io_outb(0x1F5, (unsigned char)(sector >> 24));
    io_outb(0x1F2, (unsigned char)(count & 0x0F));
    io_outb(0x1F3, (unsigned char)(sector >> 0));
    io_outb(0x1F4, (unsigned char)(sector >> 4));
    io_outb(0x1F5, (unsigned char)(sector >> 8));
    io_outb(0x1F6, 0x40 | device->secondary);
    io_outb(0x1F7, 0x24);

    unsigned int i;
    unsigned short *out = (unsigned short *)buffer;

    for (i = 0; i < count; i++)
    {

        if (!wait(device))
            return 0;

        unsigned int i;

        for (i = 0; i < 256; i++)
            *out++ = io_inw(0x1F0);

    }

    return count * 512;

}

static unsigned int ata_device_write_lba48(struct ata_device *device, unsigned int sector, unsigned int count, void *buffer)
{

    return 0;

}

static void ata_read_identity(struct ata_device *device)
{

    if (!wait(device))
        return;

    unsigned short buffer[256];

    unsigned int i;

    for (i = 0; i < 256; i++)
        buffer[i] = io_inw(device->data);

    device->lba28Max = (buffer[60] << 16) | buffer[61];

    unsigned int lba48 = buffer[83] & (1 << 10);

    if (lba48)
    {

        device->lba48MaxLow = (buffer[100] << 16) | buffer[101];
        device->lba48MaxHigh = (buffer[102] << 16) | buffer[103];

    }

}

void ata_device_init(struct ata_device *device, unsigned int type, unsigned int secondary, unsigned int control, unsigned int data)
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

    if (device->type == ATA_DEVICE_TYPE_ATA)
        ata_read_identity(device);

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

static void handle_irq_primary()
{

    log_write("[ata] irq 0x0E\n");

}

static void handle_irq_secondary()
{

    log_write("[ata] irq 0x0F\n");

}

void ata_bus_init(struct ata_bus *bus)
{

    modules_bus_init(&bus->base, ATA_BUS_TYPE);
    bus->find_device = ata_bus_find_device;

}

void init()
{

    ata_bus_init(&busPrimary);
    modules_register_bus(&busPrimary.base);

    kernel_register_irq(0x0E, handle_irq_primary);

    ata_bus_init(&busSecondary);
    modules_register_bus(&busSecondary.base);

    kernel_register_irq(0x0F, handle_irq_secondary);

    unsigned int type;
    
    if ((type = detect(ATA_PRIMARY_MASTER_CONTROL, ATA_PRIMARY_MASTER_DATA, ATA_DEVICE_PRIMARY)))
    {

        ata_device_init(&devices[0], type, ATA_DEVICE_PRIMARY, ATA_PRIMARY_MASTER_CONTROL, ATA_PRIMARY_MASTER_DATA);

        modules_register_device(&devices[0].base);

    }

    if ((type = detect(ATA_PRIMARY_SLAVE_CONTROL, ATA_PRIMARY_SLAVE_DATA, ATA_DEVICE_PRIMARY)))
    {

        ata_device_init(&devices[1], type, ATA_DEVICE_PRIMARY, ATA_PRIMARY_SLAVE_CONTROL, ATA_PRIMARY_SLAVE_DATA);

        modules_register_device(&devices[1].base);

    }

    if ((type = detect(ATA_SECONDARY_MASTER_CONTROL, ATA_SECONDARY_MASTER_DATA, ATA_DEVICE_SECONDARY)))
    {

        ata_device_init(&devices[2], type, ATA_DEVICE_SECONDARY, ATA_SECONDARY_MASTER_CONTROL, ATA_SECONDARY_MASTER_DATA);

        modules_register_device(&devices[2].base);

    }

    if ((type = detect(ATA_SECONDARY_SLAVE_CONTROL, ATA_SECONDARY_SLAVE_DATA, ATA_DEVICE_SECONDARY)))
    {

        ata_device_init(&devices[3], type, ATA_DEVICE_SECONDARY, ATA_SECONDARY_SLAVE_CONTROL, ATA_SECONDARY_SLAVE_DATA);

        modules_register_device(&devices[3].base);

    }

}

void destroy()
{

}

