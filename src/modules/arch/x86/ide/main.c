#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/arch/x86/io/io.h>
#include <modules/arch/x86/pci/pci.h>
#include "ide.h"

#define CONTROLATAPIEJECT               0x1B
#define CONTROLPIO28READ                0x20
#define CONTROLPIO48READ                0x24
#define CONTROLDMA48READ                0x25
#define CONTROLPIO28WRITE               0x30
#define CONTROLPIO48WRITE               0x34
#define CONTROLDMA48WRITE               0x35
#define CONTROLATAPI                    0xA0
#define CONTROLIDATAPI                  0xA1
#define CONTROLATAPIREAD                0xA8
#define CONTROLDMA28READ                0xC8
#define CONTROLDMA28WRITE               0xCA
#define CONTROLIDATA                    0xEC
#define DATAFEATURE                     0x01
#define DATACOUNT0                      0x02
#define DATALBA0                        0x03
#define DATALBA1                        0x04
#define DATALBA2                        0x05
#define DATASELECT                      0x06
#define DATACOMMAND                     0x07
#define DATACOUNT1                      0x08
#define DATALBA3                        0x09
#define DATALBA4                        0x0A
#define DATALBA5                        0x0B
#define IRQPRIMARY                      0x0E
#define IRQSECONDARY                    0x0F
#define STATUSERROR                     0x01
#define STATUSDRQ                       0x08
#define STATUSSRV                       0x10
#define STATUSDF                        0x20
#define STATUSRDY                       0x40
#define STATUSBUSY                      0x80
#define IDTYPE                          0x00
#define IDSERIAL                        0x0A
#define IDMODEL                         0x1B
#define IDCAP                           0x31
#define IDVALID                         0x35
#define IDLBA28MAX                      0x3C
#define IDSUPPORT                       0x53
#define IDLBA48MAX                      0x64

static struct base_driver driver;
static struct base_bus bus;
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

static void sleep(unsigned short control)
{

    io_inb(control);
    io_inb(control);
    io_inb(control);
    io_inb(control);

}

static void select(unsigned short data, unsigned short control, unsigned char operation, unsigned int slave)
{

    io_outb(data + DATASELECT, operation | slave << 4);
    sleep(control);

}

static void setcommand(unsigned short data, unsigned char command)
{

    io_outb(data + DATACOMMAND, command);

}

static void setlba(unsigned short data, unsigned char count, unsigned char lba0, unsigned char lba1, unsigned char lba2)
{

    io_outb(data + DATACOUNT0, count);
    io_outb(data + DATALBA0, lba0);
    io_outb(data + DATALBA1, lba1);
    io_outb(data + DATALBA2, lba2);

}

static void setlba2(unsigned short data, unsigned char count, unsigned char lba3, unsigned char lba4, unsigned char lba5)
{

    io_outb(data + DATACOUNT1, count);
    io_outb(data + DATALBA3, lba3);
    io_outb(data + DATALBA4, lba4);
    io_outb(data + DATALBA5, lba5);

}

static void setpio28(unsigned short data, unsigned short control, unsigned int slave, unsigned int sector, unsigned int count, unsigned char command)
{

    select(data, control, 0xE0 | ((sector >> 24) & 0x0F), slave);
    setlba(data, count, sector, sector >> 8, sector >> 16);
    setcommand(data, command);

}

static void setpio48(unsigned short data, unsigned short control, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count, unsigned char command)
{

    select(data, control, 0x40, slave);
    setlba(data, count, sectorlow, sectorlow >> 8, sectorlow >> 16);
    setlba2(data, count >> 8, sectorhigh, sectorhigh >> 8, sectorhigh >> 16);
    setcommand(data, command);

}

unsigned char ide_getstatus(unsigned int id)
{

    unsigned short data = getdata(id);

    return io_inb(data + DATACOMMAND);

}

unsigned short ide_getirq(unsigned int id)
{

    return IRQPRIMARY;

}

unsigned int ide_rblock(unsigned int id, void *buffer, unsigned int count)
{

    unsigned short data = getdata(id);
    unsigned short *out = buffer;
    unsigned int i;

    for (i = 0; i < 256; i++)
        *out++ = io_inw(data);

    return 512;

}

unsigned int ide_wblock(unsigned int id, void *buffer, unsigned int count)
{

    unsigned short data = getdata(id);
    unsigned short *out = buffer;
    unsigned int i;

    for (i = 0; i < 256; i++)
        io_outw(data, *out++);

    return 512;

}

void ide_rpio28(unsigned int id, unsigned int slave, unsigned int sector, unsigned int count)
{

    unsigned short data = getdata(id);
    unsigned short control = getcontrol(id);

    setpio28(data, control, slave, sector, count, CONTROLPIO28READ);

}

void ide_wpio28(unsigned int id, unsigned int slave, unsigned int sector, unsigned int count)
{

    unsigned short data = getdata(id);
    unsigned short control = getcontrol(id);

    setpio28(data, control, slave, sector, count, CONTROLPIO28WRITE);

}

void ide_rpio48(unsigned int id, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count)
{

    unsigned short data = getdata(id);
    unsigned short control = getcontrol(id);

    setpio48(data, control, slave, sectorlow, sectorhigh, count, CONTROLPIO48READ);

}

void ide_wpio48(unsigned int id, unsigned int slave, unsigned int sectorlow, unsigned int sectorhigh, unsigned int count)
{

    unsigned short data = getdata(id);
    unsigned short control = getcontrol(id);

    setpio48(data, control, slave, sectorlow, sectorhigh, count, CONTROLPIO48WRITE);

}

static void bus_setup(void)
{

}

static unsigned int bus_next(unsigned int id)
{

    if (id == 0)
        return IDE_ATA;

    return 0;

}

static void driver_init(void)
{

    base_initbus(&bus, IDE_BUS, "ide", bus_setup, bus_next);

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

    base_registerbus(&bus);

}

static void driver_detach(unsigned int id)
{

    base_unregisterbus(&bus);

}

void module_init(void)
{

    base_initdriver(&driver, "ide", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

