#include <fudge.h>
#include <kernel.h>
#include <base/base.h>
#include <arch/x86/io/io.h>
#include <arch/x86/pci/pci.h>
#include "ide.h"

enum ide_control
{

    IDE_CONTROL_ATAPIEJECT              = 0x1B,
    IDE_CONTROL_PIO28READ               = 0x20,
    IDE_CONTROL_PIO48READ               = 0x24,
    IDE_CONTROL_DMA48READ               = 0x25,
    IDE_CONTROL_PIO28WRITE              = 0x30,
    IDE_CONTROL_PIO48WRITE              = 0x34,
    IDE_CONTROL_DMA48WRITE              = 0x35,
    IDE_CONTROL_ATAPI                   = 0xA0,
    IDE_CONTROL_IDATAPI                 = 0xA1,
    IDE_CONTROL_ATAPIREAD               = 0xA8,
    IDE_CONTROL_DMA28READ               = 0xC8,
    IDE_CONTROL_DMA28WRITE              = 0xCA,
    IDE_CONTROL_IDATA                   = 0xEC

};

enum ide_data
{

    IDE_DATA_FEATURE                    = 0x01,
    IDE_DATA_COUNT0                     = 0x02,
    IDE_DATA_LBA0                       = 0x03,
    IDE_DATA_LBA1                       = 0x04,
    IDE_DATA_LBA2                       = 0x05,
    IDE_DATA_SELECT                     = 0x06,
    IDE_DATA_COMMAND                    = 0x07,
    IDE_DATA_COUNT1                     = 0x08,
    IDE_DATA_LBA3                       = 0x09,
    IDE_DATA_LBA4                       = 0x0A,
    IDE_DATA_LBA5                       = 0x0B

};

enum ide_irq
{

    IDE_IRQ_PRIMARY                     = 0x0E,
    IDE_IRQ_SECONDARY                   = 0x0F

};

enum ide_status
{

    IDE_STATUS_ERROR                    = 0x01,
    IDE_STATUS_DRQ                      = 0x08,
    IDE_STATUS_SRV                      = 0x10,
    IDE_STATUS_DF                       = 0x20,
    IDE_STATUS_RDY                      = 0x40,
    IDE_STATUS_BUSY                     = 0x80

};

enum ide_id
{

    IDE_ID_TYPE                         = 0x00,
    IDE_ID_SERIAL                       = 0x0A,
    IDE_ID_MODEL                        = 0x1B,
    IDE_ID_CAP                          = 0x31,
    IDE_ID_VALID                        = 0x35,
    IDE_ID_LBA28MAX                     = 0x3C,
    IDE_ID_SUPPORT                      = 0x53,
    IDE_ID_LBA48MAX                     = 0x64

};

struct ide_bus
{

    struct base_bus base;
    unsigned short control;
    unsigned short data;
    unsigned int busmaster;

};

static struct base_driver driver;
static struct ide_bus p;
static struct ide_bus s;

static void wait(struct ide_bus *bus)
{

    while (io_inb(bus->data + IDE_DATA_COMMAND) & IDE_STATUS_BUSY);

}

static void sleep(struct ide_bus *bus)
{

    io_inb(bus->control);
    io_inb(bus->control);
    io_inb(bus->control);
    io_inb(bus->control);

}

static void select(struct ide_bus *bus, unsigned char operation, unsigned int slave)
{

    io_outb(bus->data + IDE_DATA_SELECT, operation | slave << 4);
    sleep(bus);

}

static void setlba(struct ide_bus *bus, unsigned char count, unsigned char lba0, unsigned char lba1, unsigned char lba2)
{

    io_outb(bus->data + IDE_DATA_COUNT0, count);
    io_outb(bus->data + IDE_DATA_LBA0, lba0);
    io_outb(bus->data + IDE_DATA_LBA1, lba1);
    io_outb(bus->data + IDE_DATA_LBA2, lba2);

}

static void setlba2(struct ide_bus *bus, unsigned char count, unsigned char lba3, unsigned char lba4, unsigned char lba5)
{

    io_outb(bus->data + IDE_DATA_COUNT1, count);
    io_outb(bus->data + IDE_DATA_LBA3, lba3);
    io_outb(bus->data + IDE_DATA_LBA4, lba4);
    io_outb(bus->data + IDE_DATA_LBA5, lba5);

}

static void setcommand(struct ide_bus *bus, unsigned char command)
{

    io_outb(bus->data + IDE_DATA_COMMAND, command);

}

unsigned int ide_getstatus(struct base_bus *bus)
{

    struct ide_bus *b = (struct ide_bus *)bus;

    return io_inb(b->data + IDE_DATA_COMMAND);

}

static unsigned int rblocks(struct ide_bus *bus, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = buffer;

    for (i = 0; i < count; i++)
    {

        unsigned int j;

        sleep(bus);
        wait(bus);

        for (j = 0; j < 256; j++)
            *out++ = io_inw(bus->data);

    }

    return i;

}

static unsigned int wblocks(struct ide_bus *bus, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = buffer;

    for (i = 0; i < count; i++)
    {

        unsigned int j;

        sleep(bus);
        wait(bus);

        for (j = 0; j < 256; j++)
            io_outw(bus->data, *out++);

    }

    return i;

}

unsigned int ide_rblock(struct base_bus *bus, unsigned int count, void *buffer)
{

    struct ide_bus *b = (struct ide_bus *)bus;
    unsigned int i;
    unsigned short *out = buffer;

    for (i = 0; i < 256; i++)
        *out++ = io_inw(b->data);

    return 512;

}

unsigned int ide_wblock(struct base_bus *bus, unsigned int count, void *buffer)
{

    struct ide_bus *b = (struct ide_bus *)bus;
    unsigned int i;
    unsigned short *out = buffer;

    for (i = 0; i < 256; i++)
        io_outw(b->data, *out++);

    return 512;

}

unsigned int ide_rlba28(struct base_bus *bus, unsigned int slave, unsigned int sector, unsigned int count, void *buffer)
{

    struct ide_bus *b = (struct ide_bus *)bus;

    select(b, 0xE0 | ((sector >> 24) & 0x0F), slave);
    setlba(b, count, sector, sector >> 8, sector >> 16);
    setcommand(b, IDE_CONTROL_PIO28READ);

    return rblocks(b, count, buffer) * 512;

}

void ide_rlba28a(struct base_bus *bus, unsigned int slave, unsigned int sector, unsigned int count, void *buffer)
{

    struct ide_bus *b = (struct ide_bus *)bus;

    select(b, 0xE0 | ((sector >> 24) & 0x0F), slave);
    setlba(b, count, sector, sector >> 8, sector >> 16);
    setcommand(b, IDE_CONTROL_PIO28READ);

}

unsigned int ide_wlba28(struct base_bus *bus, unsigned int slave, unsigned int sector, unsigned int count, void *buffer)
{

    struct ide_bus *b = (struct ide_bus *)bus;

    select(b, 0xE0 | ((sector >> 24) & 0x0F), slave);
    setlba(b, count, sector, sector >> 8, sector >> 16);
    setcommand(b, IDE_CONTROL_PIO28WRITE);

    return wblocks(b, count, buffer) * 512;

}

void ide_wlba28a(struct base_bus *bus, unsigned int slave, unsigned int sector, unsigned int count, void *buffer)
{

    struct ide_bus *b = (struct ide_bus *)bus;

    select(b, 0xE0 | ((sector >> 24) & 0x0F), slave);
    setlba(b, count, sector, sector >> 8, sector >> 16);
    setcommand(b, IDE_CONTROL_PIO28WRITE);

}

unsigned int ide_rlba48(struct base_bus *bus, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    struct ide_bus *b = (struct ide_bus *)bus;

    select(b, 0x40, slave);
    setlba2(b, count >> 8, sectorhigh, sectorhigh >> 8, sectorhigh >> 16);
    setlba(b, count, sectorlow, sectorlow >> 8, sectorlow >> 16);
    setcommand(b, IDE_CONTROL_PIO48READ);

    return rblocks(b, count, buffer) * 512;

}

void ide_rlba48a(struct base_bus *bus, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    struct ide_bus *b = (struct ide_bus *)bus;

    select(b, 0x40, slave);
    setlba2(b, count >> 8, sectorhigh, sectorhigh >> 8, sectorhigh >> 16);
    setlba(b, count, sectorlow, sectorlow >> 8, sectorlow >> 16);
    setcommand(b, IDE_CONTROL_PIO48READ);
    sleep(b);

}

unsigned int ide_wlba48(struct base_bus *bus, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    struct ide_bus *b = (struct ide_bus *)bus;

    select(b, 0x40, slave);
    setlba2(b, count >> 8, sectorhigh, sectorhigh >> 8, sectorhigh >> 16);
    setlba(b, count, sectorlow, sectorlow >> 8, sectorlow >> 16);
    setcommand(b, IDE_CONTROL_PIO48WRITE);

    return wblocks(b, count, buffer) * 512;

}

void ide_wlba48a(struct base_bus *bus, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    struct ide_bus *b = (struct ide_bus *)bus;

    select(b, 0x40, slave);
    setlba2(b, count >> 8, sectorhigh, sectorhigh >> 8, sectorhigh >> 16);
    setlba(b, count, sectorlow, sectorlow >> 8, sectorlow >> 16);
    setcommand(b, IDE_CONTROL_PIO48WRITE);
    sleep(b);

}

/*
static unsigned int detect(struct ide_bus *bus, unsigned int slave)
{

    unsigned char buffer[512];
    unsigned char status;
    unsigned short lba;

    select(bus, 0xA0, slave);
    setlba(bus, 0, 0, 0, 0);
    setcommand(bus, IDE_CONTROL_IDATA);
    rblocks(bus, 1, buffer);

    status = io_inb(bus->data + IDE_DATA_COMMAND);

    if (!status)
        return 0;

    wait(bus);

    lba = (io_inb(bus->data + IDE_DATA_LBA2) << 8) | io_inb(bus->data + IDE_DATA_LBA1);

    if (lba == 0x0000)
        return IDE_DEVICE_TYPE_ATA;

    if (lba == 0xEB14)
        return IDE_DEVICE_TYPE_ATAPI;

    if (lba == 0xC33C)
        return IDE_DEVICE_TYPE_SATA;

    if (lba == 0x9669)
        return IDE_DEVICE_TYPE_SATAPI;

    return 0;

}
*/

static void bus_setup(struct base_bus *self)
{

}

static unsigned int bus_next_p(struct base_bus *self, unsigned int id)
{

    if (id == 0)
        return IDE_DEVICE_TYPE_ATA;

    return 0;

}

static unsigned int bus_next_s(struct base_bus *self, unsigned int id)
{

    return 0;

}

static unsigned short bus_irq(struct base_bus *self, unsigned int id)
{

    return (id) ? IDE_IRQ_SECONDARY : IDE_IRQ_PRIMARY;

}

static unsigned int driver_match(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PCI_BUS_TYPE)
        return 0;

    return pci_inb(bus, id, PCI_CONFIG_CLASS) == PCI_CLASS_STORAGE && pci_inb(bus, id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_STORAGE_IDE;

}

static void driver_attach(struct base_bus *bus, unsigned int id)
{

    unsigned int bar0 = pci_ind(bus, id, PCI_CONFIG_BAR0);
    unsigned int bar1 = pci_ind(bus, id, PCI_CONFIG_BAR1);
    unsigned int bar2 = pci_ind(bus, id, PCI_CONFIG_BAR2);
    unsigned int bar3 = pci_ind(bus, id, PCI_CONFIG_BAR3);
    unsigned int bar4 = pci_ind(bus, id, PCI_CONFIG_BAR4);

    p.data = (bar0 & 0xFFFFFFFC) + 0x1F0 * (!bar0);
    p.control = (bar1 & 0xFFFFFFFC) + 0x3F6 * (!bar1);
    p.busmaster = (bar4 & 0xFFFFFFFC) + 0;
    s.data = (bar2 & 0xFFFFFFFC) + 0x170 * (!bar2);
    s.control = (bar3 & 0xFFFFFFFC) + 0x376 * (!bar3);
    s.busmaster = (bar4 & 0xFFFFFFFC) + 8;

    base_registerbus(&p.base);
    base_registerbus(&s.base);

}

static void driver_detach(struct base_bus *bus, unsigned int id)
{

    base_unregisterbus(&p.base);
    base_unregisterbus(&s.base);

}

void init()
{

    base_initdriver(&driver, "ide", driver_match, driver_attach, driver_detach);
    base_initbus(&p.base, IDE_BUS_TYPE, "ide0", bus_setup, bus_next_p, bus_irq);
    base_initbus(&s.base, IDE_BUS_TYPE, "ide1", bus_setup, bus_next_s, bus_irq);
    base_registerdriver(&driver);

}

void destroy()
{

    base_unregisterdriver(&driver);

}

