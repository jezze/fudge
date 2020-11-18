#include <fudge.h>
#include <kernel.h>
#include <modules/base/bus.h>
#include <modules/base/driver.h>
#include <modules/system/system.h>
#include <modules/arch/x86/io/io.h>
#include <modules/arch/x86/pci/pci.h>
#include "ide.h"

#define REG_FEATURE                     0x01
#define REG_COUNT0                      0x02
#define REG_LBA0                        0x03
#define REG_LBA1                        0x04
#define REG_LBA2                        0x05
#define REG_SELECT                      0x06
#define REG_COMMAND                     0x07
#define REG_COMMAND_ATAPIEJECT          0x1B
#define REG_COMMAND_PIO28READ           0x20
#define REG_COMMAND_PIO48READ           0x24
#define REG_COMMAND_DMA48READ           0x25
#define REG_COMMAND_PIO28WRITE          0x30
#define REG_COMMAND_PIO48WRITE          0x34
#define REG_COMMAND_DMA48WRITE          0x35
#define REG_COMMAND_ATAPI               0xA0
#define REG_COMMAND_IDATAPI             0xA1
#define REG_COMMAND_ATAPIREAD           0xA8
#define REG_COMMAND_DMA28READ           0xC8
#define REG_COMMAND_DMA28WRITE          0xCA
#define REG_COMMAND_IDATA               0xEC
#define REG_COUNT1                      0x08
#define REG_LBA3                        0x09
#define REG_LBA4                        0x0A
#define REG_LBA5                        0x0B
#define REG_STATUS_ERROR                0x01
#define REG_STATUS_DRQ                  0x08
#define REG_STATUS_SRV                  0x10
#define REG_STATUS_DF                   0x20
#define REG_STATUS_RDY                  0x40
#define REG_STATUS_BUSY                 0x80
#define IRQ_PRIMARY                     0x0E
#define IRQ_SECONDARY                   0x0F
#define ID_TYPE                         0x00
#define ID_SERIAL                       0x0A
#define ID_MODEL                        0x1B
#define ID_CAP                          0x31
#define ID_VALID                        0x35
#define ID_LBA28MAX                     0x3C
#define ID_SUPPORT                      0x53
#define ID_LBA48MAX                     0x64

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

    io_outb(data + REG_SELECT, operation | slave << 4);
    sleep(control);

}

static void setcommand(unsigned short data, unsigned char command)
{

    io_outb(data + REG_COMMAND, command);

}

static void setlba(unsigned short data, unsigned char count, unsigned char lba0, unsigned char lba1, unsigned char lba2)
{

    io_outb(data + REG_COUNT0, count);
    io_outb(data + REG_LBA0, lba0);
    io_outb(data + REG_LBA1, lba1);
    io_outb(data + REG_LBA2, lba2);

}

static void setlba2(unsigned short data, unsigned char count, unsigned char lba3, unsigned char lba4, unsigned char lba5)
{

    io_outb(data + REG_COUNT1, count);
    io_outb(data + REG_LBA3, lba3);
    io_outb(data + REG_LBA4, lba4);
    io_outb(data + REG_LBA5, lba5);

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

    return io_inb(data + REG_COMMAND);

}

unsigned short ide_getirq(unsigned int id)
{

    return IRQ_PRIMARY;

}

unsigned int ide_rblock(unsigned int id, void *buffer)
{

    unsigned short data = getdata(id);
    unsigned short *out = buffer;
    unsigned int i;

    for (i = 0; i < 256; i++)
        *out++ = io_inw(data);

    return 1;

}

unsigned int ide_wblock(unsigned int id, void *buffer)
{

    unsigned short data = getdata(id);
    unsigned short *out = buffer;
    unsigned int i;

    for (i = 0; i < 256; i++)
        io_outw(data, *out++);

    return 1;

}

void ide_rpio28(unsigned int id, unsigned int slave, unsigned int count, unsigned int sector)
{

    unsigned short data = getdata(id);
    unsigned short control = getcontrol(id);

    setpio28(data, control, slave, sector, count, REG_COMMAND_PIO28READ);

}

void ide_wpio28(unsigned int id, unsigned int slave, unsigned int count, unsigned int sector)
{

    unsigned short data = getdata(id);
    unsigned short control = getcontrol(id);

    setpio28(data, control, slave, sector, count, REG_COMMAND_PIO28WRITE);

}

void ide_rpio48(unsigned int id, unsigned int slave, unsigned int count, unsigned int sector)
{

    unsigned short data = getdata(id);
    unsigned short control = getcontrol(id);

    setpio48(data, control, slave, sector, 0, count, REG_COMMAND_PIO48READ);

}

void ide_wpio48(unsigned int id, unsigned int slave, unsigned int count, unsigned int sector)
{

    unsigned short data = getdata(id);
    unsigned short control = getcontrol(id);

    setpio48(data, control, slave, sector, 0, count, REG_COMMAND_PIO48WRITE);

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

static void driver_init(unsigned int id)
{

    base_initbus(&bus, IDE_BUS, "ide", bus_setup, bus_next);

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_STORAGE && pci_inb(id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_STORAGE_IDE;

}

static void driver_reset(unsigned int id)
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

}

static void driver_attach(unsigned int id)
{

    base_registerbus(&bus);

}

static void driver_detach(unsigned int id)
{

    base_unregisterbus(&bus);

}

void module_init(void)
{

    base_initdriver(&driver, "ide", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

