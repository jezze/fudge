#include <fudge.h>
#include <net.h>
#include <kernel.h>
#include <modules/base/driver.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/apic/apic.h>
#include <modules/arch/x86/io/io.h>
#include <modules/arch/x86/pci/pci.h>

#define VENDOR                          0x10EC
#define DEVICE                          0x8139
#define REG_IDR0                        0x00
#define REG_IDR1                        0x01
#define REG_IDR2                        0x02
#define REG_IDR3                        0x03
#define REG_IDR4                        0x04
#define REG_IDR5                        0x05
#define REG_MAR0                        0x08
#define REG_MAR1                        0x09
#define REG_MAR2                        0x0A
#define REG_MAR3                        0x0B
#define REG_MAR4                        0x0C
#define REG_MAR5                        0x0D
#define REG_MAR6                        0x0E
#define REG_MAR7                        0x0F
#define REG_TSD0                        0x10
#define REG_TSD1                        0x14
#define REG_TSD2                        0x18
#define REG_TSD3                        0x1C
#define REG_TSAD0                       0x20
#define REG_TSAD1                       0x24
#define REG_TSAD2                       0x28
#define REG_TSAD3                       0x2C
#define REG_RBSTART                     0x30
#define REG_ERBCR                       0x34
#define REG_ERSR                        0x36
#define REG_CR                          0x37
#define REG_CR_REMPTY                   (1 << 0)
#define REG_CR_TENABLE                  (1 << 2)
#define REG_CR_RENABLE                  (1 << 3)
#define REG_CR_RESET                    (1 << 4)
#define REG_CAPR                        0x38
#define REG_CBR                         0x3A
#define REG_IMR                         0x3C
#define REG_ISR                         0x3E
#define REG_ISR_ROK                     (1 << 0)
#define REG_ISR_RER                     (1 << 1)
#define REG_ISR_TOK                     (1 << 2)
#define REG_ISR_TER                     (1 << 3)
#define REG_ISR_RXO                     (1 << 4)
#define REG_ISR_PUN                     (1 << 5)
#define REG_ISR_RXU                     (1 << 6)
#define REG_TCR                         0x40
#define REG_RCR                         0x44
#define REG_RCR_AAP                     (1 << 0)
#define REG_RCR_APM                     (1 << 1)
#define REG_RCR_AM                      (1 << 2)
#define REG_RCR_AB                      (1 << 3)
#define REG_RCR_AR                      (1 << 4)
#define REG_RCR_AER                     (1 << 5)
#define REG_RCR_WRAP                    (1 << 7)
#define REG_TCTR                        0x48
#define REG_MPC                         0x4C
#define REG_9346CR                      0x50
#define REG_CONFIG0                     0x51
#define REG_CONFIG1                     0x52
#define REG_TIMERINT                    0x54
#define REG_MSR                         0x58
#define REG_CONFIG3                     0x59
#define REG_CONFIG4                     0x5A
#define REG_MULINT                      0x5C
#define REG_RERID                       0x5E
#define REG_TSAD                        0x60
#define REG_BMCR                        0x62
#define REG_BMSR                        0x64
#define REG_ANAR                        0x66
#define REG_ANLPAR                      0x68
#define REG_ANER                        0x6A
#define REG_DIS                         0x6C
#define HEADER_ROK                      (1 << 0)
#define HEADER_FAE                      (1 << 1)
#define HEADER_CRC                      (1 << 2)
#define HEADER_LONG                     (1 << 3)
#define HEADER_RUNT                     (1 << 4)
#define HEADER_ISE                      (1 << 5)
#define HEADER_BAR                      (1 << 13)
#define HEADER_PAM                      (1 << 14)
#define HEADER_MAR                      (1 << 15)

struct header
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

    io_outb(io + REG_CONFIG1, 0x00);

}

static void reset(void)
{

    io_outb(io + REG_CR, REG_CR_RESET);

    while (io_inb(io + REG_CR) & REG_CR_RESET);

}

static void enable(void)
{

    io_outb(io + REG_CR, REG_CR_RENABLE | REG_CR_TENABLE);

}

static void setintflags(unsigned short flags)
{

    io_outw(io + REG_IMR, flags);

}

static void setrx(void)
{

    io_outd(io + REG_RBSTART, (unsigned int)rx);
    io_outd(io + REG_RCR, REG_RCR_AAP | REG_RCR_APM | REG_RCR_AM | REG_RCR_AB | REG_RCR_WRAP);

}

static void settx(void)
{

    io_outd(io + REG_TSAD0, (unsigned int)tx0);
    io_outd(io + REG_TSAD1, (unsigned int)tx1);
    io_outd(io + REG_TSAD2, (unsigned int)tx2);
    io_outd(io + REG_TSAD3, (unsigned int)tx3);

}

static void handleirq(unsigned int irq)
{

    unsigned short status = io_inw(io + REG_ISR);

    if (status & REG_ISR_ROK)
    {

        unsigned short end = io_inw(io + REG_CBR);
        unsigned short limit = (end < rxp) ? 8192 : end;

        while (rxp < limit)
        {

            struct header *header = (struct header *)(rx + rxp);

            ethernet_notify(&ethernetinterface, header + 1, header->length);

            rxp += (header->length + 4 + 3) & ~3;

            if (rxp >= 8192)
            {

                rxp -= 8192;
                limit = end;

            }

            io_outw(io + REG_CAPR, rxp - 16);

        }

        io_outw(io + REG_ISR, REG_ISR_ROK);

    }

    if (status & REG_ISR_RER)
    {

        io_outw(io + REG_ISR, REG_ISR_RER);

    }

    if (status & REG_ISR_TOK)
    {

        io_outw(io + REG_ISR, REG_ISR_TOK);

    }

    if (status & REG_ISR_TER)
    {

        io_outw(io + REG_ISR, REG_ISR_TER);

    }

    if (status & REG_ISR_RXO)
    {

        io_outw(io + REG_ISR, REG_ISR_RXO);

    }

    if (status & REG_ISR_PUN)
    {

        io_outw(io + REG_ISR, REG_ISR_PUN);

    }

    if (status & REG_ISR_RXU)
    {

        io_outw(io + REG_ISR, REG_ISR_RXU);

    }

}

static unsigned int ethernetinterface_send(void *buffer, unsigned int count)
{

    unsigned int status = (0x3F << 16) | (count & 0x1FFF);

    switch (txp)
    {

    case 0:
        buffer_write(tx0, 0x800, buffer, count, 0);
        io_outd(io + REG_TSD0, status);

        break;

    case 1:
        buffer_write(tx1, 0x800, buffer, count, 0);
        io_outd(io + REG_TSD1, status);

        break;

    case 2:
        buffer_write(tx2, 0x800, buffer, count, 0);
        io_outd(io + REG_TSD2, status);

        break;

    case 3:
        buffer_write(tx3, 0x800, buffer, count, 0);
        io_outd(io + REG_TSD3, status);

        break;

    }

    txp++;
    txp %= 4;

    return count;

}

static unsigned int ethernetinterface_readaddr(void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char address[ETHERNET_ADDRSIZE];

    address[0] = io_inb(io + REG_IDR0);
    address[1] = io_inb(io + REG_IDR1);
    address[2] = io_inb(io + REG_IDR2);
    address[3] = io_inb(io + REG_IDR3);
    address[4] = io_inb(io + REG_IDR4);
    address[5] = io_inb(io + REG_IDR5);

    return buffer_read(buffer, count, address, ETHERNET_ADDRSIZE, offset);

}

static unsigned int ethernetinterface_notifydata(struct list *links, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    if (event == EVENT_DATA)
    {

        ethernetinterface_send(data, count);

        return count;

    }

    return 0;

}

static void driver_init(unsigned int id)
{

    ethernet_initinterface(&ethernetinterface, id);

    ethernetinterface.addr.operations.read = ethernetinterface_readaddr;
    ethernetinterface.data.operations.notify = ethernetinterface_notifydata;

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inw(id, PCI_CONFIG_VENDOR) == VENDOR && pci_inw(id, PCI_CONFIG_DEVICE) == DEVICE;

}

static void driver_reset(unsigned int id)
{

    io = pci_inw(id, PCI_CONFIG_BAR0) & ~1;
    mmio = pci_ind(id, PCI_CONFIG_BAR1);

    poweron();
    reset();
    setintflags(REG_ISR_ROK | REG_ISR_RER | REG_ISR_TOK | REG_ISR_TER | REG_ISR_RXO | REG_ISR_PUN | REG_ISR_RXU);
    setrx();
    settx();
    enable();
    pci_setmaster(id);

}

static void driver_attach(unsigned int id)
{

    unsigned short irq = pci_getirq(id);

    ethernet_registerinterface(&ethernetinterface);
    pic_setroutine(irq, handleirq);
    apic_setroutine(irq, handleirq);

}

static void driver_detach(unsigned int id)
{

    unsigned short irq = pci_getirq(id);

    ethernet_unregisterinterface(&ethernetinterface);
    pic_unsetroutine(irq);
    apic_unsetroutine(irq);

}

void module_init(void)
{

    base_initdriver(&driver, "rtl8139", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

