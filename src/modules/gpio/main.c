#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/acpi/acpi.h>
#include <modules/apic/apic.h>
#include <modules/bus.h>
#include <modules/pci/pci.h>
#include "gpio.h"

#define P2SB                            (0x80000000 | (0x1F << 11) | (1 << 8))
#define P2SB_HIDE                       0xE1
#define SBREG_PORTSIZE                  0x10000
#define VENDOR_INTEL                    0x8086
#define PADS                            8
#define STORM                           10000
#define PADBAR                          0x0C
#define DW0_RXSTATE                     (1 << 1)
#define DW0_TXDIS                       (1 << 8)
#define DW0_RXDIS                       (1 << 9)
#define DW0_PMODE                       (7 << 10)
#define DW0_ROUTES                      (0xF << 17)
#define DW0_RXINV                       (1 << 23)
#define DW0_RXEVCFG                     (3 << 25)
#define DW0_RXEVCFG_EDGE                (1 << 25)

struct group
{

    unsigned int port;
    unsigned int first;
    unsigned int count;
    unsigned int reg;

};

struct chip
{

    char *name;
    unsigned short smbus;
    unsigned long sbreg;
    unsigned int irq;
    unsigned int padsize;
    unsigned int hostswown;
    unsigned int gpiis;
    unsigned int gpiie;
    struct group *groups;
    unsigned int ngroups;

};

struct interrupt
{

    struct gpio_pad pad;
    unsigned int (*routine)(void);

};

/* groups in the order Intel's reference firmware numbers them (its GPCL table, the group in bits 16-23 of a pad like GPDI): community port, first pad in the community, pads, first HOSTSW_OWN/GPI_IS/GPI_IE register */
static struct group groupstgllp[] = {
    {0x6E, 0, 26, 0}, {0x6E, 26, 16, 1}, {0x6E, 42, 25, 2},
    {0x69, 0, 8, 0}, {0x69, 8, 9, 1},
    {0x6C, 0, 17, 0},
    {0x6D, 0, 8, 0}, {0x6D, 8, 24, 1}, {0x6D, 32, 21, 2}, {0x6D, 53, 24, 3}, {0x6D, 77, 27, 4},
    {0x6A, 0, 24, 0}, {0x6A, 24, 25, 1}, {0x6A, 49, 6, 2}, {0x6A, 55, 25, 3}, {0x6A, 80, 10, 4},
    {0x6B, 0, 15, 0}, {0x6B, 15, 91, 1}
};

static struct group groupstglh[] = {
    {0x6E, 0, 25, 0}, {0x6E, 25, 20, 1}, {0x6E, 45, 24, 2}, {0x6E, 71, 8, 3},
    {0x6D, 0, 24, 0}, {0x6D, 26, 24, 1}, {0x6D, 50, 8, 2}, {0x6D, 58, 16, 3}, {0x6D, 75, 27, 4},
    {0x6C, 0, 13, 0},
    {0x6B, 0, 13, 0}, {0x6B, 13, 24, 1}, {0x6B, 0, 0, 2},
    {0x6A, 0, 24, 0}, {0x6A, 24, 10, 1}, {0x6A, 34, 12, 2},
    {0x69, 0, 15, 0}, {0x69, 15, 9, 1}, {0x69, 24, 14, 2}
};

/* a chip is recognised by its SMBus controller, visible at 00:1F.4 on Intel PCHs; sbreg (0 for none known) and irq are used when neither the firmware (SBRG, SGIR) nor P2SB says; register offsets as Linux's pinctrl driver for the chip; Tiger Lake H is untested */
static struct chip chips[] = {
    {"Tiger Lake LP", 0xA0A3, 0xFD000000, 14, 16, 0xB0, 0x100, 0x120, groupstgllp, sizeof (groupstgllp) / sizeof (struct group)},
    {"Tiger Lake H", 0x43A3, 0, 14, 16, 0xC0, 0x100, 0x120, groupstglh, sizeof (groupstglh) / sizeof (struct group)}
};

static struct interrupt interrupts[PADS];
static struct chip *chip;
static unsigned long sbreg;
static unsigned int irq;
static unsigned int spurious;

static unsigned int readpad(unsigned long address)
{

    return *(volatile unsigned int *)address;

}

static void writepad(unsigned long address, unsigned int value)
{

    *(volatile unsigned int *)address = value;

}

static unsigned int countinterrupts(void)
{

    unsigned int count = 0;
    unsigned int i;

    for (i = 0; i < PADS; i++)
    {

        if (interrupts[i].routine)
            count++;

    }

    return count;

}

static void stopinterrupt(struct interrupt *interrupt)
{

    unsigned long ie = interrupt->pad.community + chip->gpiie + interrupt->pad.reg * 4;

    writepad(ie, readpad(ie) & ~(1 << interrupt->pad.bit));

    interrupt->routine = 0;

}

static void handleirq(unsigned int irq)
{

    unsigned int found = 0;
    unsigned int i;

    for (i = 0; i < PADS; i++)
    {

        struct interrupt *interrupt = &interrupts[i];
        unsigned long is = interrupt->pad.community + chip->gpiis + interrupt->pad.reg * 4;

        if (!interrupt->routine)
            continue;

        if (readpad(is) & (1 << interrupt->pad.bit))
        {

            /* clear first: the next announcement, even one made while the client reads, latches again and is not wiped out */
            writepad(is, 1 << interrupt->pad.bit);

            found += interrupt->routine();

        }

    }

    /* the line is shared by every pad: one that keeps firing without any client finding anything is set up wrong (pad or apic polarity), so give up on it and the clients poll */
    spurious = (found) ? 0 : spurious + 1;

    if (spurious == STORM)
    {

        for (i = 0; i < PADS; i++)
        {

            if (interrupts[i].routine)
                stopinterrupt(&interrupts[i]);

        }

        apic_unsetroutine(irq, handleirq);
        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "gpio: interrupt fires without results, turned off");

    }

}

static void stopall(struct gpio_pad *pad)
{

    unsigned int i;

    for (i = 0; i < PADS; i++)
    {

        if (interrupts[i].routine && (!pad || interrupts[i].pad.dw0 == pad->dw0))
            stopinterrupt(&interrupts[i]);

    }

    if (!countinterrupts())
        apic_unsetroutine(irq, handleirq);

}

static unsigned long readp2sb(void)
{

    unsigned long sbreg;

    /* the sideband registers sit behind the hidden P2SB device: show it, read its BAR, hide it again */
    pci_outb(P2SB, P2SB_HIDE, 0);

    sbreg = pci_ind(P2SB, PCI_CONFIG_BAR0) & 0xFFFFFFF0;

    pci_outb(P2SB, P2SB_HIDE, 1);

    return (!sbreg || sbreg == 0xFFFFFFF0) ? chip->sbreg : sbreg;

}

static struct chip *findchip(void)
{

    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_BUS)))
    {

        struct base_bus *pcibus = current->data;
        unsigned int id = 0;

        if (pcibus->type != PCI_BUS)
            continue;

        while ((id = pcibus->next(id)))
        {

            unsigned short device = pci_inw(id, PCI_CONFIG_DEVICE);
            unsigned int i;

            if (pci_inw(id, PCI_CONFIG_VENDOR) != VENDOR_INTEL)
                continue;

            for (i = 0; i < sizeof (chips) / sizeof (struct chip); i++)
            {

                if (device == chips[i].smbus)
                    return &chips[i];

            }

        }

    }

    return 0;

}

struct gpio_pad gpio_findpad(unsigned int group, unsigned int pad)
{

    struct gpio_pad gpiopad;
    struct group *g;
    unsigned long community;
    unsigned long dw0;
    unsigned int index;
    unsigned int padbar;
    unsigned int value;

    gpiopad.community = 0;
    gpiopad.dw0 = 0;
    gpiopad.reg = 0;
    gpiopad.bit = 0;

    if (!chip || group >= chip->ngroups || pad >= chip->groups[group].count)
        return gpiopad;

    g = &chip->groups[group];
    community = sbreg + (g->port << 16);
    index = g->first + pad;
    gpiopad.community = community;
    gpiopad.reg = g->reg + pad / 32;
    gpiopad.bit = pad % 32;

    arch_kmap(community, community, SBREG_PORTSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    padbar = readpad(community + PADBAR);

    if (!padbar || padbar >= SBREG_PORTSIZE - (index + 1) * chip->padsize)
        return gpiopad;

    dw0 = community + padbar + index * chip->padsize;
    value = readpad(dw0);

    /* only a GPIO input will do */
    if ((value & DW0_PMODE) || (value & DW0_RXDIS) || !(value & DW0_TXDIS))
        return gpiopad;

    gpiopad.dw0 = dw0;

    return gpiopad;

}

unsigned int gpio_islow(struct gpio_pad *pad)
{

    return !(readpad(pad->dw0) & DW0_RXSTATE);

}

unsigned int gpio_setinterrupt(struct gpio_pad *pad, unsigned int (*routine)(void))
{

    unsigned long is;
    unsigned long ie;
    unsigned long owner;
    unsigned int dw0;
    unsigned int i;

    if (!chip || !pad->dw0)
        return 0;

    is = pad->community + chip->gpiis + pad->reg * 4;
    ie = pad->community + chip->gpiie + pad->reg * 4;
    owner = pad->community + chip->hostswown + pad->reg * 4;
    dw0 = readpad(pad->dw0);

    for (i = 0; i < PADS; i++)
    {

        if (!interrupts[i].routine)
            break;

    }

    if (i == PADS)
        return 0;

    /* edge, inverted so each falling edge of an active low line latches the status once, routed to the GPIO controller's shared interrupt instead of SCI, SMI, NMI or the IOxAPIC */
    dw0 &= ~(DW0_ROUTES | DW0_RXEVCFG);
    dw0 |= DW0_RXINV | DW0_RXEVCFG_EDGE;

    writepad(pad->dw0, dw0);
    writepad(owner, readpad(owner) | (1 << pad->bit));
    writepad(is, 1 << pad->bit);

    if (!apic_setlevel(irq, 1))
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "gpio: no apic line for the GPIO interrupt");

        return 0;

    }

    if (!countinterrupts())
    {

        spurious = 0;

        apic_setroutine(irq, handleirq);

    }

    interrupts[i].pad.community = pad->community;
    interrupts[i].pad.dw0 = pad->dw0;
    interrupts[i].pad.reg = pad->reg;
    interrupts[i].pad.bit = pad->bit;
    interrupts[i].routine = routine;

    writepad(ie, readpad(ie) | (1 << pad->bit));

    return 1;

}

void gpio_unsetinterrupt(struct gpio_pad *pad)
{

    stopall(pad);

}

void module_init(void)
{

    unsigned int firmwaresbreg;
    unsigned int firmwareirq;

    chip = findchip();

    if (!chip)
        return;

    /* Intel's reference firmware keeps the sideband base and the GPIO controller's interrupt in SBRG and SGIR, 0 when it has none */
    firmwaresbreg = acpi_readfield("SBRG").value;
    firmwareirq = acpi_readfield("SGIR").value;
    sbreg = (firmwaresbreg) ? firmwaresbreg : readp2sb();
    irq = (firmwareirq) ? firmwareirq : chip->irq;

    if (!sbreg)
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "gpio: %s without a sideband base, no pads", chip->name);

        chip = 0;

        return;

    }

}

void module_unregister(void)
{

    stopall((void *)0);

}

