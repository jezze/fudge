#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <modules/ata/ata.h>

static unsigned int read_blocks(struct ata_bus *self, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = (unsigned short *)buffer;

    for (i = 0; i < count; i++)
    {

        self->sleep(self);
        self->wait(self);

        unsigned int i;

        for (i = 0; i < 256; i++)
            *out++ = io_inw(self->data);

    }

    return i;

}

static unsigned int write_blocks(struct ata_bus *self, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = (unsigned short *)buffer;

    for (i = 0; i < count; i++)
    {

        self->sleep(self);
        self->wait(self);

        unsigned int i;

        for (i = 0; i < 256; i++)
            io_outw(self->data, *out++);

    }

    return i;


}

static void sleep(struct ata_bus *self)
{

    io_inb(self->control);
    io_inb(self->control);
    io_inb(self->control);
    io_inb(self->control);

}

static void wait(struct ata_bus *self)
{

    while (io_inb(self->data + ATA_DATA_COMMAND) & ATA_STATUS_FLAG_BUSY);

}

static void select(struct ata_bus *self, unsigned char operation, unsigned int slave)
{

    io_outb(self->data + ATA_DATA_SELECT, operation | slave << 4);
    self->sleep(self);

}

static void set_lba(struct ata_bus *self, unsigned char count, unsigned char lba0, unsigned char lba1, unsigned char lba2)
{

    io_outb(self->data + ATA_DATA_COUNT0, count);
    io_outb(self->data + ATA_DATA_LBA0, lba0);
    io_outb(self->data + ATA_DATA_LBA1, lba1);
    io_outb(self->data + ATA_DATA_LBA2, lba2);

}

static void set_lba2(struct ata_bus *self, unsigned char count, unsigned char lba3, unsigned char lba4, unsigned char lba5)
{

    io_outb(self->data + ATA_DATA_COUNT1, count);
    io_outb(self->data + ATA_DATA_LBA3, lba3);
    io_outb(self->data + ATA_DATA_LBA4, lba4);
    io_outb(self->data + ATA_DATA_LBA5, lba5);

}

static void set_command(struct ata_bus *self, unsigned char command)
{

    io_outb(self->data + ATA_DATA_COMMAND, command);

}

static unsigned int detect(struct ata_bus *self, unsigned int slave)
{

    self->select(self, 0xA0, slave);
    self->set_lba(self, 0, 0, 0, 0);
    self->set_command(self, ATA_COMMAND_ID_ATA);

    unsigned char status = io_inb(self->data + ATA_DATA_COMMAND);

    if (!status)
        return 0;

    self->wait(self);

    unsigned short lba = (io_inb(self->data + ATA_DATA_LBA2) << 8) | io_inb(self->data + ATA_DATA_LBA1);

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

void ata_bus_scan(struct ata_bus *self, void (*callback)(struct ata_bus *bus, unsigned int slave, unsigned int type))
{

    unsigned int type;

    if ((type = self->detect(self, 0)))
        callback(self, 0, type);

    if ((type = self->detect(self, 1)))
        callback(self, 1, type);

}

void ata_bus_init(struct ata_bus *bus, unsigned int control, unsigned int data)
{

    modules_bus_init(&bus->base, ATA_BUS_TYPE, "ata:0");

    bus->control = control;
    bus->data = data;
    bus->sleep = sleep;
    bus->wait = wait;
    bus->select = select;
    bus->set_lba = set_lba;
    bus->set_lba2 = set_lba2;
    bus->set_command = set_command;
    bus->detect = detect;
    bus->read_blocks = read_blocks;
    bus->write_blocks = write_blocks;
    bus->scan = ata_bus_scan;

}

