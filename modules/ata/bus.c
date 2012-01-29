#include <kernel/arch/x86/io.h>
#include <kernel/irq.h>
#include <kernel/modules.h>
#include <modules/ata/ata.h>

static unsigned int ata_bus_read_blocks(struct ata_bus *bus, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = (unsigned short *)buffer;

    for (i = 0; i < count; i++)
    {

        bus->sleep(bus);
        bus->wait(bus);

        unsigned int i;

        for (i = 0; i < 256; i++)
            *out++ = io_inw(bus->data);

    }

    return i;

}

static void ata_bus_sleep(struct ata_bus *self)
{

    io_inb(self->control);
    io_inb(self->control);
    io_inb(self->control);
    io_inb(self->control);

}

static void ata_bus_wait(struct ata_bus *self)
{

    while (io_inb(self->data + ATA_DATA_COMMAND) & ATA_STATUS_FLAG_BUSY);

}

static void ata_bus_select(struct ata_bus *self, unsigned char operation, unsigned int secondary)
{

    io_outb(self->data + ATA_DATA_SELECT, operation | secondary);
    self->sleep(self);

}

static void ata_bus_set_lba(struct ata_bus *self, unsigned char count, unsigned char lba0, unsigned char lba1, unsigned char lba2)
{

    io_outb(self->data + ATA_DATA_COUNT0, count);
    io_outb(self->data + ATA_DATA_LBA0, lba0);
    io_outb(self->data + ATA_DATA_LBA1, lba1);
    io_outb(self->data + ATA_DATA_LBA2, lba2);

}

static void ata_bus_set_lba2(struct ata_bus *self, unsigned char count, unsigned char lba3, unsigned char lba4, unsigned char lba5)
{

    io_outb(self->data + ATA_DATA_COUNT1, count);
    io_outb(self->data + ATA_DATA_LBA3, lba3);
    io_outb(self->data + ATA_DATA_LBA4, lba4);
    io_outb(self->data + ATA_DATA_LBA5, lba5);

}

static void ata_bus_set_command(struct ata_bus *self, unsigned char command)
{

    io_outb(self->data + ATA_DATA_COMMAND, command);

}

static unsigned int ata_bus_detect(struct ata_bus *self, unsigned int secondary, void *buffer)
{

    self->select(self, 0xA0, secondary);
    self->set_lba(self, 0, 0, 0, 0);
    self->set_command(self, ATA_COMMAND_ID_ATA);

    unsigned char status = io_inb(self->data + ATA_DATA_COMMAND);

    if (!status)
        return 0;

    self->wait(self);

    unsigned short lba = (io_inb(self->data + ATA_DATA_LBA2) << 8) | io_inb(self->data + ATA_DATA_LBA1);

    if (lba == 0x0000)
    {

        self->read_blocks(self, 1, buffer);

        return ATA_DEVICE_TYPE_ATA;

    }

    if (lba == 0xEB14)
    {

        self->set_command(self, ATA_COMMAND_ID_ATAPI);
        self->read_blocks(self, 1, buffer);

        return ATA_DEVICE_TYPE_ATAPI;

    }

    if (lba == 0xC33C)
        return ATA_DEVICE_TYPE_SATA;

    if (lba == 0x9669)
        return ATA_DEVICE_TYPE_SATAPI;

    return 0;

}

static void handle_irq()
{

}

void ata_bus_scan(struct ata_bus *bus, void (*callback)(struct ata_bus *bus, unsigned int master, unsigned int type, void *buffer))
{

    unsigned short buffer[256];
    unsigned int type;

    if ((type = bus->detect(bus, 0 << 4, buffer)))
        callback(bus, 0 << 4, type, buffer);

    if ((type = bus->detect(bus, 1 << 4, buffer)))
        callback(bus, 1 << 4, type, buffer);

}

void ata_bus_init(struct ata_bus *bus, unsigned int control, unsigned int data, unsigned int irq)
{

    modules_bus_init(&bus->base, ATA_BUS_TYPE);

    bus->control = control;
    bus->data = data;
    bus->irq = irq;
    bus->sleep = ata_bus_sleep;
    bus->wait = ata_bus_wait;
    bus->select = ata_bus_select;
    bus->set_lba = ata_bus_set_lba;
    bus->set_lba2 = ata_bus_set_lba2;
    bus->set_command = ata_bus_set_command;
    bus->detect = ata_bus_detect;
    bus->read_blocks = ata_bus_read_blocks;
    bus->scan = ata_bus_scan;

}

