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

static struct base_driver driver;
static struct base_bus idebus;
static unsigned short pcontrol;
static unsigned short scontrol;
static unsigned short pdata;
static unsigned short sdata;
static unsigned int busmaster;

static unsigned short getcontrol(unsigned int id)
{

    return pcontrol;

}

static unsigned short getdata(unsigned int id)
{

    return pdata;

}

static void wait(unsigned short data)
{

    while (io_inb(data + IDE_DATA_COMMAND) & IDE_STATUS_BUSY);

}

static void sleep(unsigned short control)
{

    io_inb(control);
    io_inb(control);
    io_inb(control);
    io_inb(control);

}

static void select(unsigned short data, unsigned short control, unsigned char operation, unsigned int slave)
{

    io_outb(data + IDE_DATA_SELECT, operation | slave << 4);
    sleep(control);

}

static void setlba(unsigned short data, unsigned char count, unsigned char lba0, unsigned char lba1, unsigned char lba2)
{

    io_outb(data + IDE_DATA_COUNT0, count);
    io_outb(data + IDE_DATA_LBA0, lba0);
    io_outb(data + IDE_DATA_LBA1, lba1);
    io_outb(data + IDE_DATA_LBA2, lba2);

}

static void setlba2(unsigned short data, unsigned char count, unsigned char lba3, unsigned char lba4, unsigned char lba5)
{

    io_outb(data + IDE_DATA_COUNT1, count);
    io_outb(data + IDE_DATA_LBA3, lba3);
    io_outb(data + IDE_DATA_LBA4, lba4);
    io_outb(data + IDE_DATA_LBA5, lba5);

}

static void setcommand(unsigned short data, unsigned char command)
{

    io_outb(data + IDE_DATA_COMMAND, command);

}

static unsigned int rblocks(unsigned short data, unsigned short control, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = buffer;

    for (i = 0; i < count; i++)
    {

        unsigned int j;

        sleep(control);
        wait(data);

        for (j = 0; j < 256; j++)
            *out++ = io_inw(data);

    }

    return i;

}

static unsigned int wblocks(unsigned short data, unsigned short control, unsigned int count, void *buffer)
{

    unsigned int i;
    unsigned short *out = buffer;

    for (i = 0; i < count; i++)
    {

        unsigned int j;

        sleep(control);
        wait(data);

        for (j = 0; j < 256; j++)
            io_outw(data, *out++);

    }

    return i;

}

unsigned short ide_getirq(unsigned int id)
{

    return (id) ? IDE_IRQ_SECONDARY : IDE_IRQ_PRIMARY;

}

unsigned int ide_rblock(unsigned int id, unsigned int count, void *buffer)
{

    unsigned short data = getdata(id);
    unsigned short *out = buffer;
    unsigned int i;

    for (i = 0; i < 256; i++)
        *out++ = io_inw(data);

    return 512;

}

unsigned int ide_wblock(unsigned int id, unsigned int count, void *buffer)
{

    unsigned short data = getdata(id);
    unsigned short *out = buffer;
    unsigned int i;

    for (i = 0; i < 256; i++)
        io_outw(data, *out++);

    return 512;

}

unsigned int ide_rlba28(unsigned int id, unsigned int slave, unsigned int sector, unsigned int count, void *buffer)
{

    unsigned short data = getdata(id);
    unsigned short control = getcontrol(id);

    select(data, control, 0xE0 | ((sector >> 24) & 0x0F), slave);
    setlba(data, count, sector, sector >> 8, sector >> 16);
    setcommand(data, IDE_CONTROL_PIO28READ);

    return rblocks(data, control, count, buffer) * 512;

}

void ide_rlba28a(unsigned int id, unsigned int slave, unsigned int sector, unsigned int count, void *buffer)
{

    unsigned short data = getdata(id);
    unsigned short control = getcontrol(id);

    select(data, control, 0xE0 | ((sector >> 24) & 0x0F), slave);
    setlba(data, count, sector, sector >> 8, sector >> 16);
    setcommand(data, IDE_CONTROL_PIO28READ);

}

unsigned int ide_wlba28(unsigned int id, unsigned int slave, unsigned int sector, unsigned int count, void *buffer)
{

    unsigned short data = getdata(id);
    unsigned short control = getcontrol(id);

    select(data, control, 0xE0 | ((sector >> 24) & 0x0F), slave);
    setlba(data, count, sector, sector >> 8, sector >> 16);
    setcommand(data, IDE_CONTROL_PIO28WRITE);

    return wblocks(data, control, count, buffer) * 512;

}

void ide_wlba28a(unsigned int id, unsigned int slave, unsigned int sector, unsigned int count, void *buffer)
{

    unsigned short data = getdata(id);
    unsigned short control = getcontrol(id);

    select(data, control, 0xE0 | ((sector >> 24) & 0x0F), slave);
    setlba(data, count, sector, sector >> 8, sector >> 16);
    setcommand(data, IDE_CONTROL_PIO28WRITE);

}

unsigned int ide_rlba48(unsigned int id, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    unsigned short data = getdata(id);
    unsigned short control = getcontrol(id);

    select(data, control, 0x40, slave);
    setlba2(data, count >> 8, sectorhigh, sectorhigh >> 8, sectorhigh >> 16);
    setlba(data, count, sectorlow, sectorlow >> 8, sectorlow >> 16);
    setcommand(data, IDE_CONTROL_PIO48READ);

    return rblocks(data, control, count, buffer) * 512;

}

void ide_rlba48a(unsigned int id, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    unsigned short data = getdata(id);
    unsigned short control = getcontrol(id);

    select(data, control, 0x40, slave);
    setlba2(data, count >> 8, sectorhigh, sectorhigh >> 8, sectorhigh >> 16);
    setlba(data, count, sectorlow, sectorlow >> 8, sectorlow >> 16);
    setcommand(data, IDE_CONTROL_PIO48READ);
    sleep(control);

}

unsigned int ide_wlba48(unsigned int id, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    unsigned short data = getdata(id);
    unsigned short control = getcontrol(id);

    select(data, control, 0x40, slave);
    setlba2(data, count >> 8, sectorhigh, sectorhigh >> 8, sectorhigh >> 16);
    setlba(data, count, sectorlow, sectorlow >> 8, sectorlow >> 16);
    setcommand(data, IDE_CONTROL_PIO48WRITE);

    return wblocks(data, control, count, buffer) * 512;

}

void ide_wlba48a(unsigned int id, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, void *buffer)
{

    unsigned short data = getdata(id);
    unsigned short control = getcontrol(id);

    select(data, control, 0x40, slave);
    setlba2(data, count >> 8, sectorhigh, sectorhigh >> 8, sectorhigh >> 16);
    setlba(data, count, sectorlow, sectorlow >> 8, sectorlow >> 16);
    setcommand(data, IDE_CONTROL_PIO48WRITE);
    sleep(control);

}

static void bus_setup()
{

}

static unsigned int bus_next(unsigned int id)
{

    if (id == 0)
        return IDE_DEVICE_TYPE_ATA;

    return 0;

}

static void driver_init()
{

    base_initbus(&idebus, IDE_BUS_TYPE, "ide", bus_setup, bus_next);

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_STORAGE && pci_inb(id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_STORAGE_IDE;

}

static void driver_attach(unsigned int id)
{

    unsigned int bar0 = pci_ind(id, PCI_CONFIG_BAR0);
    unsigned int bar1 = pci_ind(id, PCI_CONFIG_BAR1);
    unsigned int bar2 = pci_ind(id, PCI_CONFIG_BAR2);
    unsigned int bar3 = pci_ind(id, PCI_CONFIG_BAR3);
    unsigned int bar4 = pci_ind(id, PCI_CONFIG_BAR4);

    pdata = (bar0 & 0xFFFFFFFC) + 0x1F0 * (!bar0);
    pcontrol = (bar1 & 0xFFFFFFFC) + 0x3F6 * (!bar1);
    sdata = (bar2 & 0xFFFFFFFC) + 0x170 * (!bar2);
    scontrol = (bar3 & 0xFFFFFFFC) + 0x376 * (!bar3);
    busmaster = (bar4 & 0xFFFFFFFC) + 8;

    base_registerbus(&idebus);

}

static void driver_detach(unsigned int id)
{

    base_unregisterbus(&idebus);

}

void module_init()
{

    base_initdriver(&driver, "ide", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register()
{

    base_registerdriver(&driver, PCI_BUS_TYPE);

}

void module_unregister()
{

    base_unregisterdriver(&driver, PCI_BUS_TYPE);

}

