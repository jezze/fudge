#include <fudge.h>
#include <kernel.h>
#include <modules/bus.h>
#include <modules/driver.h>
#include <modules/io/io.h>
#include <modules/pci/pci.h>
#include "ide.h"

#define REG_FEATURE                     0x01
#define REG_COUNT                       0x02
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
#define LIMIT_POLL                      100000
#define CHANNELS                        2
#define BM_COMMAND                      0x00
#define BM_COMMAND_START                0x01
#define BM_COMMAND_READ                 0x08
#define BM_STATUS                       0x02
#define BM_STATUS_ERROR                 0x02
#define BM_STATUS_INTERRUPT             0x04
#define BM_PRD                          0x04
#define PRDS                            2
#define PRD_END                         0x80000000
#define PRD_BOUNDARY                    0x10000

static struct base_driver driver;
static struct base_bus bus;
static unsigned short pcontrol;
static unsigned short scontrol;
static unsigned short pdata;
static unsigned short sdata;
static unsigned short pmaster;
static unsigned short smaster;
static unsigned int prdmemory[CHANNELS * PRDS * 2 + 4];
static unsigned int owners[CHANNELS];
static struct spinlock spinlock;

static unsigned int getchannel(unsigned int id)
{

    switch (id)
    {

    case IDE_SM:
    case IDE_SS:
        return 1;

    }

    return 0;

}

static unsigned short getcontrol(unsigned int id)
{

    switch (id)
    {

    case IDE_PM:
    case IDE_PS:
        return pcontrol;

    case IDE_SM:
    case IDE_SS:
        return scontrol;

    }

    return 0;

}

static unsigned short getdata(unsigned int id)
{

    switch (id)
    {

    case IDE_PM:
    case IDE_PS:
        return pdata;

    case IDE_SM:
    case IDE_SS:
        return sdata;

    }

    return 0;

}

static unsigned short getmaster(unsigned int id)
{

    switch (id)
    {

    case IDE_PM:
    case IDE_PS:
        return pmaster;

    case IDE_SM:
    case IDE_SS:
        return smaster;

    }

    return 0;

}

/* each channel's table is 16-byte aligned, so it never crosses a 64 KB boundary */
static unsigned int *getprd(unsigned int id)
{

    unsigned int *tables = (unsigned int *)(((unsigned long)prdmemory + 15) & ~15UL);

    return tables + getchannel(id) * PRDS * 2;

}

static void setpio28(unsigned int id, unsigned int sector, unsigned int count, unsigned char command)
{

    unsigned short data = getdata(id);
    unsigned short control = getcontrol(id);
    unsigned int select = 0xE0 | ((sector >> 24) & 0x0F);

    switch (id)
    {

    case IDE_PS:
    case IDE_SS:
        select |= 0x10;

        break;

    }

    io_outb(data + REG_SELECT, select);
    io_inb(control);
    io_inb(control);
    io_inb(control);
    io_inb(control);
    io_outb(data + REG_COUNT, count);
    io_outb(data + REG_LBA0, sector);
    io_outb(data + REG_LBA1, sector >> 8);
    io_outb(data + REG_LBA2, sector >> 16);
    io_outb(data + REG_COMMAND, command);

}

static void setpio48(unsigned int id, unsigned int sector, unsigned int count, unsigned char command)
{

    unsigned short data = getdata(id);
    unsigned short control = getcontrol(id);
    unsigned int select = 0x40;

    switch (id)
    {

    case IDE_PS:
    case IDE_SS:
        select |= 0x10;

        break;

    }

    io_outb(data + REG_SELECT, select);
    io_inb(control);
    io_inb(control);
    io_inb(control);
    io_inb(control);
    io_outb(data + REG_COUNT, count >> 8);
    io_outb(data + REG_LBA0, sector >> 24);
    io_outb(data + REG_LBA1, 0);
    io_outb(data + REG_LBA2, 0);
    io_outb(data + REG_COUNT, count);
    io_outb(data + REG_LBA0, sector);
    io_outb(data + REG_LBA1, sector >> 8);
    io_outb(data + REG_LBA2, sector >> 16);
    io_outb(data + REG_COMMAND, command);

}

unsigned char ide_getstatus(unsigned int id)
{

    unsigned short data = getdata(id);

    return io_inb(data + REG_COMMAND);

}

static unsigned int identify(unsigned int id, void *buffer, unsigned char command)
{

    unsigned short data = getdata(id);
    unsigned short control = getcontrol(id);
    unsigned int select = 0xA0;

    switch (id)
    {

    case IDE_PS:
    case IDE_SS:
        select |= 0x10;

        break;

    }

    io_outb(data + REG_SELECT, select);
    io_inb(control);
    io_inb(control);
    io_inb(control);
    io_inb(control);

    if (io_inb(data + REG_COMMAND) != 0xFF)
    {

        unsigned char status;
        unsigned int i = 0;

        io_outb(data + REG_COUNT, 0);
        io_outb(data + REG_LBA0, 0);
        io_outb(data + REG_LBA1, 0);
        io_outb(data + REG_LBA2, 0);
        io_outb(data + REG_COMMAND, command);

        do
        {

            status = io_inb(data + REG_COMMAND);

        }
        while ((status & REG_STATUS_BUSY) && ++i < LIMIT_POLL);

        if (!status || (status & REG_STATUS_BUSY))
            return 0;

        if (command == REG_COMMAND_IDATA && (io_inb(data + REG_LBA1) || io_inb(data + REG_LBA2)))
            return 0;

        while (!(status & (REG_STATUS_DRQ | REG_STATUS_ERROR)) && ++i < LIMIT_POLL)
            status = io_inb(data + REG_COMMAND);

        if (!(status & REG_STATUS_DRQ) || (status & REG_STATUS_ERROR))
            return 0;

        return ide_rblock(id, buffer);

    }

    return 0;

}

unsigned int ide_identify(unsigned int id, void *buffer)
{

    return identify(id, buffer, REG_COMMAND_IDATA);

}

unsigned int ide_identifypacket(unsigned int id, void *buffer)
{

    return identify(id, buffer, REG_COMMAND_IDATAPI);

}

unsigned short ide_getirq(unsigned int id)
{

    switch (id)
    {

    case IDE_PM:
    case IDE_PS:
        return IRQ_PRIMARY;

    case IDE_SM:
    case IDE_SS:
        return IRQ_SECONDARY;

    }

    return 0;

}

unsigned int ide_wait(unsigned int id)
{

    unsigned short control = getcontrol(id);
    unsigned char status;
    unsigned int i = 0;

    io_inb(control);
    io_inb(control);
    io_inb(control);
    io_inb(control);

    do
    {

        status = ide_getstatus(id);

    }
    while ((status & REG_STATUS_BUSY) && ++i < LIMIT_POLL);

    return !(status & REG_STATUS_BUSY) && (status & REG_STATUS_DRQ) && !(status & REG_STATUS_ERROR);

}

unsigned int ide_rblock(unsigned int id, void *buffer)
{

    unsigned short data = getdata(id);
    unsigned short *out = buffer;
    unsigned int i;

    for (i = 0; i < 256; i++)
        out[i] = io_inw(data);

    return 1;

}

unsigned int ide_wblock(unsigned int id, void *buffer)
{

    unsigned short data = getdata(id);
    unsigned short *out = buffer;
    unsigned int i;

    for (i = 0; i < 256; i++)
        io_outw(data, out[i]);

    return 1;

}

unsigned int ide_lock(unsigned int id)
{

    unsigned int channel = getchannel(id);
    unsigned int locked;

    spinlock_acquire(&spinlock);

    locked = !owners[channel] || owners[channel] == id;

    if (locked)
        owners[channel] = id;

    spinlock_release(&spinlock);

    return locked;

}

void ide_unlock(unsigned int id)
{

    unsigned int channel = getchannel(id);

    spinlock_acquire(&spinlock);

    if (owners[channel] == id)
        owners[channel] = 0;

    spinlock_release(&spinlock);

}

unsigned int ide_packet(unsigned int id, void *packet, unsigned int limit, unsigned int dma)
{

    unsigned short data = getdata(id);
    unsigned short control = getcontrol(id);
    unsigned int select = 0xA0;

    switch (id)
    {

    case IDE_PS:
    case IDE_SS:
        select |= 0x10;

        break;

    }

    io_outb(data + REG_SELECT, select);
    io_inb(control);
    io_inb(control);
    io_inb(control);
    io_inb(control);
    io_outb(data + REG_FEATURE, (dma) ? 1 : 0);
    io_outb(data + REG_LBA1, limit);
    io_outb(data + REG_LBA2, limit >> 8);
    io_outb(data + REG_COMMAND, REG_COMMAND_ATAPI);

    if (ide_wait(id))
    {

        unsigned short *words = packet;
        unsigned int i;

        for (i = 0; i < IDE_PACKETSIZE / 2; i++)
            io_outw(data, words[i]);

        return 1;

    }

    return 0;

}

unsigned char ide_getreason(unsigned int id)
{

    unsigned short data = getdata(id);

    return io_inb(data + REG_COUNT);

}

unsigned int ide_getbytecount(unsigned int id)
{

    unsigned short data = getdata(id);

    return io_inb(data + REG_LBA1) | (io_inb(data + REG_LBA2) << 8);

}

void ide_rdata(unsigned int id, void *buffer, unsigned int count)
{

    unsigned short data = getdata(id);
    unsigned short *out = buffer;
    unsigned int i;

    for (i = 0; i < count / 2; i++)
        out[i] = io_inw(data);

}

unsigned int ide_hasdma(unsigned int id)
{

    return getmaster(id) != 0;

}

/* address is physical; an entry may not cross a 64 KB boundary */
unsigned int ide_setdma(unsigned int id, unsigned long address, unsigned int count, unsigned int read)
{

    unsigned short master = getmaster(id);

    if (master && count)
    {

        unsigned int *prd = getprd(id);
        unsigned int i;

        for (i = 0; i < PRDS && count; i++)
        {

            unsigned int length = PRD_BOUNDARY - (address & (PRD_BOUNDARY - 1));

            if (length > count)
                length = count;

            prd[i * 2] = address;
            prd[i * 2 + 1] = length & 0xFFFF;
            address += length;
            count -= length;

        }

        if (count)
            return 0;

        prd[i * 2 - 1] |= PRD_END;

        io_outb(master + BM_COMMAND, 0);
        io_outb(master + BM_STATUS, BM_STATUS_ERROR | BM_STATUS_INTERRUPT);
        io_outd(master + BM_PRD, (unsigned long)prd);
        io_outb(master + BM_COMMAND, (read) ? BM_COMMAND_READ : 0);

        return 1;

    }

    return 0;

}

void ide_startdma(unsigned int id)
{

    unsigned short master = getmaster(id);

    io_outb(master + BM_COMMAND, io_inb(master + BM_COMMAND) | BM_COMMAND_START);

}

/* IDE_DMA_BUSY while the transfer runs, then the engine is stopped and the result returned */
unsigned int ide_checkdma(unsigned int id)
{

    unsigned short master = getmaster(id);
    unsigned char status = io_inb(master + BM_STATUS);

    if (!(status & BM_STATUS_INTERRUPT))
        return IDE_DMA_BUSY;

    io_outb(master + BM_COMMAND, io_inb(master + BM_COMMAND) & ~BM_COMMAND_START);
    io_outb(master + BM_STATUS, status & (BM_STATUS_ERROR | BM_STATUS_INTERRUPT));

    return (status & BM_STATUS_ERROR) ? IDE_DMA_FAILED : IDE_DMA_DONE;

}

void ide_rdma48(unsigned int id, unsigned int count, unsigned int sector)
{

    setpio48(id, sector, count, REG_COMMAND_DMA48READ);

}

void ide_wdma48(unsigned int id, unsigned int count, unsigned int sector)
{

    setpio48(id, sector, count, REG_COMMAND_DMA48WRITE);

}

void ide_rpio28(unsigned int id, unsigned int count, unsigned int sector)
{

    setpio28(id, sector, count, REG_COMMAND_PIO28READ);

}

void ide_wpio28(unsigned int id, unsigned int count, unsigned int sector)
{

    setpio28(id, sector, count, REG_COMMAND_PIO28WRITE);

}

void ide_rpio48(unsigned int id, unsigned int count, unsigned int sector)
{

    setpio48(id, sector, count, REG_COMMAND_PIO48READ);

}

void ide_wpio48(unsigned int id, unsigned int count, unsigned int sector)
{

    setpio48(id, sector, count, REG_COMMAND_PIO48WRITE);

}

static void bus_setup(void)
{

}

static unsigned int bus_next(unsigned int id)
{

    if (id == 0)
        return IDE_PM;

    if (id == IDE_PM)
        return IDE_PS;

    if (id == IDE_PS)
        return IDE_SM;

    if (id == IDE_SM)
        return IDE_SS;

    return 0;

}

static void driver_init(unsigned int id)
{

    base_initbus(&bus, IDE_BUS, "ide", bus_setup, bus_next);

}

static unsigned int driver_match(unsigned int type, unsigned int id)
{

    return type == PCI_BUS && pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_STORAGE && pci_inb(id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_STORAGE_IDE;

}

static void driver_reset(unsigned int id)
{

    unsigned long bar0 = pci_getbar(id, 0);
    unsigned long bar1 = pci_getbar(id, 1);
    unsigned long bar2 = pci_getbar(id, 2);
    unsigned long bar3 = pci_getbar(id, 3);
    unsigned long bar4 = pci_getbar(id, 4);

    pdata = bar0 + 0x1F0 * (!bar0);
    pcontrol = bar1 + 0x3F6 * (!bar1);
    sdata = bar2 + 0x170 * (!bar2);
    scontrol = bar3 + 0x376 * (!bar3);

    if (bar4)
    {

        pmaster = bar4;
        smaster = bar4 + 8;

        pci_setmaster(id);

    }

    io_outb(pcontrol, 0);
    io_outb(scontrol, 0);

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

    base_registerdriver(&driver);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver);

}

