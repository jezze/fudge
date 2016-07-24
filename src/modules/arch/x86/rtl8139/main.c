#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/io/io.h>
#include <modules/arch/x86/pci/pci.h>

#define PCIVENDOR                       0x10EC
#define PCIDEVICE                       0x8139
#define REGISTERIDR0                    0x00
#define REGISTERIDR1                    0x01
#define REGISTERIDR2                    0x02
#define REGISTERIDR3                    0x03
#define REGISTERIDR4                    0x04
#define REGISTERIDR5                    0x05
#define REGISTERMAR0                    0x08
#define REGISTERMAR1                    0x09
#define REGISTERMAR2                    0x0A
#define REGISTERMAR3                    0x0B
#define REGISTERMAR4                    0x0C
#define REGISTERMAR5                    0x0D
#define REGISTERMAR6                    0x0E
#define REGISTERMAR7                    0x0F
#define REGISTERTSD0                    0x10
#define REGISTERTSD1                    0x14
#define REGISTERTSD2                    0x18
#define REGISTERTSD3                    0x1C
#define REGISTERTSAD0                   0x20
#define REGISTERTSAD1                   0x24
#define REGISTERTSAD2                   0x28
#define REGISTERTSAD3                   0x2C
#define REGISTERRBSTART                 0x30
#define REGISTERERBCR                   0x34
#define REGISTERERSR                    0x36
#define REGISTERCR                      0x37
#define REGISTERCAPR                    0x38
#define REGISTERCBR                     0x3A
#define REGISTERIMR                     0x3C
#define REGISTERISR                     0x3E
#define REGISTERTCR                     0x40
#define REGISTERRCR                     0x44
#define REGISTERTCTR                    0x48
#define REGISTERMPC                     0x4C
#define REGISTER9346CR                  0x50
#define REGISTERCONFIG0                 0x51
#define REGISTERCONFIG1                 0x52
#define REGISTERTIMERINT                0x54
#define REGISTERMSR                     0x58
#define REGISTERCONFIG3                 0x59
#define REGISTERCONFIG4                 0x5A
#define REGISTERMULINT                  0x5C
#define REGISTERRERID                   0x5E
#define REGISTERTSAD                    0x60
#define REGISTERBMCR                    0x62
#define REGISTERBMSR                    0x64
#define REGISTERANAR                    0x66
#define REGISTERANLPAR                  0x68
#define REGISTERANER                    0x6A
#define REGISTERDIS                     0x6C
#define RCRAAP                          (1 << 0)
#define RCRAPM                          (1 << 1)
#define RCRAM                           (1 << 2)
#define RCRAB                           (1 << 3)
#define RCRAR                           (1 << 4)
#define RCRAER                          (1 << 5)
#define RCRWRAP                         (1 << 7)
#define CRREMPTY                        (1 << 0)
#define CRTENABLE                       (1 << 2)
#define CRRENABLE                       (1 << 3)
#define CRRESET                         (1 << 4)
#define ISRROK                          (1 << 0)
#define ISRRER                          (1 << 1)
#define ISRTOK                          (1 << 2)
#define ISRTER                          (1 << 3)
#define HEADERFLAGROK                   (1 << 0)
#define HEADERFLAGFAE                   (1 << 1)
#define HEADERFLAGCRC                   (1 << 2)
#define HEADERFLAGLONG                  (1 << 3)
#define HEADERFLAGRUNT                  (1 << 4)
#define HEADERFLAGISE                   (1 << 5)
#define HEADERFLAGBAR                   (1 << 13)
#define HEADERFLAGPAM                   (1 << 14)
#define HEADERFLAGMAR                   (1 << 15)

struct rtl8139_header
{

    unsigned short flags;
    unsigned short length;

};

static struct base_driver driver;
static struct ethernet_interface ethernetinterface;
static unsigned short io;
static unsigned int mmio;
static unsigned char rx[0x2600];
static unsigned char tx0[0x800];
static unsigned char tx1[0x800];
static unsigned char tx2[0x800];
static unsigned char tx3[0x800];
static unsigned short rxp;
static unsigned short txp;

static void poweron(void)
{

    io_outb(io + REGISTERCONFIG1, 0x00);

}

static void reset(void)
{

    io_outb(io + REGISTERCR, CRRESET);

    while (io_inb(io + REGISTERCR) & CRRESET);

}

static void enable(void)
{

    io_outb(io + REGISTERCR, CRRENABLE | CRTENABLE);

}

static void setintflags(unsigned short flags)
{

    io_outw(io + REGISTERIMR, flags);

}

static void setrx(void)
{

    io_outd(io + REGISTERRBSTART, (unsigned long)rx);
    io_outd(io + REGISTERRCR, RCRAAP | RCRAPM | RCRAM | RCRAB | RCRWRAP);

}

static void settx(void)
{

    io_outd(io + REGISTERTSAD0, (unsigned long)tx0);
    io_outd(io + REGISTERTSAD1, (unsigned long)tx1);
    io_outd(io + REGISTERTSAD2, (unsigned long)tx2);
    io_outd(io + REGISTERTSAD3, (unsigned long)tx3);

}

static void handleirq(unsigned int irq)
{

    unsigned short status = io_inw(io + REGISTERISR);

    if (status & ISRROK)
    {

        struct rtl8139_header *header = (struct rtl8139_header *)(rx + rxp);

        ethernet_notify(&ethernetinterface, header->length, header + 1);

        rxp += (header->length + 4 + 3) & ~3;

        io_outw(io + REGISTERCAPR, rxp - 16);
        io_outw(io + REGISTERISR, ISRROK);

    }

    if (status & ISRTOK)
    {

        io_outw(io + REGISTERISR, ISRTOK);

    }

}

static unsigned int ethernetinterface_send(unsigned int count, void *buffer)
{

    unsigned int status = (0x3F << 16) | (count & 0x1FFF);

    switch (txp)
    {

    case 0:
        memory_write(tx0, 0x800, buffer, count, 0);
        io_outd(io + REGISTERTSD0, status);

        break;

    case 1:
        memory_write(tx1, 0x800, buffer, count, 0);
        io_outd(io + REGISTERTSD1, status);

        break;

    case 2:
        memory_write(tx2, 0x800, buffer, count, 0);
        io_outd(io + REGISTERTSD2, status);

        break;

    case 3:
        memory_write(tx3, 0x800, buffer, count, 0);
        io_outd(io + REGISTERTSD3, status);

        break;

    }

    txp++;
    txp %= 4;

    return count;

}

static void driver_init(void)
{

    ethernet_initinterface(&ethernetinterface, driver.name, ethernetinterface_send);

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inw(id, PCI_CONFIG_VENDOR) == PCIVENDOR && pci_inw(id, PCI_CONFIG_DEVICE) == PCIDEVICE;

}

static void driver_attach(unsigned int id)
{

    io = pci_inw(id, PCI_CONFIG_BAR0) & ~1;
    mmio = pci_ind(id, PCI_CONFIG_BAR1);

    poweron();
    reset();
    setintflags(ISRROK | ISRTOK);
    setrx();
    settx();
    enable();
    pci_setmaster(id);

    ethernetinterface.haddress[0] = io_inb(io + REGISTERIDR0);
    ethernetinterface.haddress[1] = io_inb(io + REGISTERIDR1);
    ethernetinterface.haddress[2] = io_inb(io + REGISTERIDR2);
    ethernetinterface.haddress[3] = io_inb(io + REGISTERIDR3);
    ethernetinterface.haddress[4] = io_inb(io + REGISTERIDR4);
    ethernetinterface.haddress[5] = io_inb(io + REGISTERIDR5);

    ethernet_registerinterface(&ethernetinterface, id);
    pic_setroutine(pci_getirq(id), handleirq);

}

static void driver_detach(unsigned int id)
{

    ethernet_unregisterinterface(&ethernetinterface);
    pic_unsetroutine(pci_getirq(id));

}

void module_init(void)
{

    base_initdriver(&driver, "rtl8139", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

