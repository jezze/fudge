#include <fudge.h>
#include <kernel.h>
#include <modules/base/driver.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/io/io.h>
#include <modules/arch/x86/pci/pci.h>

#define VENDOR                          0x10EC
#define DEVICE                          0x8139
#define R_IDR0                          0x00
#define R_IDR1                          0x01
#define R_IDR2                          0x02
#define R_IDR3                          0x03
#define R_IDR4                          0x04
#define R_IDR5                          0x05
#define R_MAR0                          0x08
#define R_MAR1                          0x09
#define R_MAR2                          0x0A
#define R_MAR3                          0x0B
#define R_MAR4                          0x0C
#define R_MAR5                          0x0D
#define R_MAR6                          0x0E
#define R_MAR7                          0x0F
#define R_TSD0                          0x10
#define R_TSD1                          0x14
#define R_TSD2                          0x18
#define R_TSD3                          0x1C
#define R_TSAD0                         0x20
#define R_TSAD1                         0x24
#define R_TSAD2                         0x28
#define R_TSAD3                         0x2C
#define R_RBSTART                       0x30
#define R_ERBCR                         0x34
#define R_ERSR                          0x36
#define R_CR                            0x37
#define R_CAPR                          0x38
#define R_CBR                           0x3A
#define R_IMR                           0x3C
#define R_ISR                           0x3E
#define R_TCR                           0x40
#define R_RCR                           0x44
#define R_TCTR                          0x48
#define R_MPC                           0x4C
#define R_9346CR                        0x50
#define R_CONFIG0                       0x51
#define R_CONFIG1                       0x52
#define R_TIMERINT                      0x54
#define R_MSR                           0x58
#define R_CONFIG3                       0x59
#define R_CONFIG4                       0x5A
#define R_MULINT                        0x5C
#define R_RERID                         0x5E
#define R_TSAD                          0x60
#define R_BMCR                          0x62
#define R_BMSR                          0x64
#define R_ANAR                          0x66
#define R_ANLPAR                        0x68
#define R_ANER                          0x6A
#define R_DIS                           0x6C
#define F_RCR_AAP                       (1 << 0)
#define F_RCR_APM                       (1 << 1)
#define F_RCR_AM                        (1 << 2)
#define F_RCR_AB                        (1 << 3)
#define F_RCR_AR                        (1 << 4)
#define F_RCR_AER                       (1 << 5)
#define F_RCR_WRAP                      (1 << 7)
#define F_CR_REMPTY                     (1 << 0)
#define F_CR_TENABLE                    (1 << 2)
#define F_CR_RENABLE                    (1 << 3)
#define F_CR_RESET                      (1 << 4)
#define F_ISR_ROK                       (1 << 0)
#define F_ISR_RER                       (1 << 1)
#define F_ISR_TOK                       (1 << 2)
#define F_ISR_TER                       (1 << 3)
#define F_ISR_RXO                       (1 << 4)
#define F_ISR_PUN                       (1 << 5)
#define F_ISR_RXU                       (1 << 6)
#define HEADERFLAGROK                   (1 << 0)
#define HEADERFLAGFAE                   (1 << 1)
#define HEADERFLAGCRC                   (1 << 2)
#define HEADERFLAGLONG                  (1 << 3)
#define HEADERFLAGRUNT                  (1 << 4)
#define HEADERFLAGISE                   (1 << 5)
#define HEADERFLAGBAR                   (1 << 13)
#define HEADERFLAGPAM                   (1 << 14)
#define HEADERFLAGMAR                   (1 << 15)

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

    io_outb(io + R_CONFIG1, 0x00);

}

static void reset(void)
{

    io_outb(io + R_CR, F_CR_RESET);

    while (io_inb(io + R_CR) & F_CR_RESET);

}

static void enable(void)
{

    io_outb(io + R_CR, F_CR_RENABLE | F_CR_TENABLE);

}

static void setintflags(unsigned short flags)
{

    io_outw(io + R_IMR, flags);

}

static void setrx(void)
{

    io_outd(io + R_RBSTART, (unsigned int)rx);
    io_outd(io + R_RCR, F_RCR_AAP | F_RCR_APM | F_RCR_AM | F_RCR_AB | F_RCR_WRAP);

}

static void settx(void)
{

    io_outd(io + R_TSAD0, (unsigned int)tx0);
    io_outd(io + R_TSAD1, (unsigned int)tx1);
    io_outd(io + R_TSAD2, (unsigned int)tx2);
    io_outd(io + R_TSAD3, (unsigned int)tx3);

}

static void handleirq(unsigned int irq)
{

    unsigned short status = io_inw(io + R_ISR);

    if (status & F_ISR_ROK)
    {

        unsigned short end = io_inw(io + R_CBR);
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

            io_outw(io + R_CAPR, rxp - 16);

        }

        io_outw(io + R_ISR, F_ISR_ROK);

    }

    if (status & F_ISR_RER)
    {

        io_outw(io + R_ISR, F_ISR_RER);

    }

    if (status & F_ISR_TOK)
    {

        io_outw(io + R_ISR, F_ISR_TOK);

    }

    if (status & F_ISR_TER)
    {

        io_outw(io + R_ISR, F_ISR_TER);

    }

    if (status & F_ISR_RXO)
    {

        io_outw(io + R_ISR, F_ISR_RXO);

    }

    if (status & F_ISR_PUN)
    {

        io_outw(io + R_ISR, F_ISR_PUN);

    }

    if (status & F_ISR_RXU)
    {

        io_outw(io + R_ISR, F_ISR_RXU);

    }

}

static unsigned int ethernetinterface_matchaddress(void *buffer, unsigned int count)
{

    unsigned char address[ETHERNET_ADDRSIZE];

    address[0] = io_inb(io + R_IDR0);
    address[1] = io_inb(io + R_IDR1);
    address[2] = io_inb(io + R_IDR2);
    address[3] = io_inb(io + R_IDR3);
    address[4] = io_inb(io + R_IDR4);
    address[5] = io_inb(io + R_IDR5);

    return memory_match(address, buffer, count);

}

static unsigned int ethernetinterface_send(void *buffer, unsigned int count)
{

    unsigned int status = (0x3F << 16) | (count & 0x1FFF);

    switch (txp)
    {

    case 0:
        memory_write(tx0, 0x800, buffer, count, 0);
        io_outd(io + R_TSD0, status);

        break;

    case 1:
        memory_write(tx1, 0x800, buffer, count, 0);
        io_outd(io + R_TSD1, status);

        break;

    case 2:
        memory_write(tx2, 0x800, buffer, count, 0);
        io_outd(io + R_TSD2, status);

        break;

    case 3:
        memory_write(tx3, 0x800, buffer, count, 0);
        io_outd(io + R_TSD3, status);

        break;

    }

    txp++;
    txp %= 4;

    return count;

}

static unsigned int ethernetinterface_readaddr(struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char address[ETHERNET_ADDRSIZE];

    address[0] = io_inb(io + R_IDR0);
    address[1] = io_inb(io + R_IDR1);
    address[2] = io_inb(io + R_IDR2);
    address[3] = io_inb(io + R_IDR3);
    address[4] = io_inb(io + R_IDR4);
    address[5] = io_inb(io + R_IDR5);

    return memory_read(buffer, count, address, ETHERNET_ADDRSIZE, offset);

}

static void driver_init(unsigned int id)
{

    ethernet_initinterface(&ethernetinterface, id, ethernetinterface_matchaddress, ethernetinterface_send);

    ethernetinterface.addr.operations.read = ethernetinterface_readaddr;

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
    setintflags(F_ISR_ROK | F_ISR_RER | F_ISR_TOK | F_ISR_TER | F_ISR_RXO | F_ISR_PUN | F_ISR_RXU);
    setrx();
    settx();
    enable();
    pci_setmaster(id);

}

static void driver_attach(unsigned int id)
{

    ethernet_registerinterface(&ethernetinterface);
    pic_setroutine(pci_getirq(id), handleirq);

}

static void driver_detach(unsigned int id)
{

    ethernet_unregisterinterface(&ethernetinterface);
    pic_unsetroutine(pci_getirq(id));

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

