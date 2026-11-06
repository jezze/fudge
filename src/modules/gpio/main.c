#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/apic/apic.h>
#include <modules/pci/pci.h>
#include "gpio.h"

#define P2SB                            (0x80000000 | (0x1F << 11) | (1 << 8))
#define P2SB_HIDE                       0xE1
#define SBREG_DEFAULT                   0xFD000000
#define SBREG_PORTSIZE                  0x10000
#define IRQ                             14
#define PADS                            8
#define STORM                           10000
#define PADBAR                          0x0C
#define PADSIZE                         16
#define PADCFGLOCK                      0x80
#define HOSTSWOWN                       0xB0
#define GPIIS                           0x100
#define GPIIE                           0x120
#define DW0_RXSTATE                     (1 << 1)
#define DW0_TXDIS                       (1 << 8)
#define DW0_RXDIS                       (1 << 9)
#define DW0_PMODE                       (7 << 10)
#define DW0_ROUTES                      (0xF << 17)
#define DW0_RXINV                       (1 << 23)
#define DW0_RXEVCFG                     (3 << 25)
#define DW0_RXEVCFG_EDGE                (1 << 25)

struct interrupt
{

    struct gpio_pad pad;
    unsigned int (*routine)(void);
    unsigned int spurious;

};

static struct interrupt interrupts[PADS];

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

    unsigned long ie = interrupt->pad.community + GPIIE + interrupt->pad.group * 4;

    writepad(ie, readpad(ie) & ~(1 << interrupt->pad.bit));

    interrupt->routine = 0;

}

static void handleirq(unsigned int irq)
{

    unsigned int i;

    for (i = 0; i < PADS; i++)
    {

        struct interrupt *interrupt = &interrupts[i];
        unsigned long is = interrupt->pad.community + GPIIS + interrupt->pad.group * 4;
        unsigned int found = 0;

        if (!interrupt->routine)
            continue;

        if (readpad(is) & (1 << interrupt->pad.bit))
        {

            /* clear first: the next announcement, even one made while the client reads, latches again and is not wiped out */
            writepad(is, 1 << interrupt->pad.bit);

            found = interrupt->routine();

        }

        /* a line that keeps firing without the client finding anything is set up wrong (pad or apic polarity): give up on it, the client polls */
        interrupt->spurious = (found) ? 0 : interrupt->spurious + 1;

        if (interrupt->spurious == STORM)
        {

            stopinterrupt(interrupt);

            if (!countinterrupts())
                apic_unsetroutine(IRQ, handleirq);

            debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "gpio: interrupt fires without results, turned off");

        }

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
        apic_unsetroutine(IRQ, handleirq);

}

static unsigned long getsbreg(void)
{

    unsigned long sbreg;

    /* the sideband registers sit behind the hidden P2SB device: show it, read its BAR, hide it again */
    pci_outb(P2SB, P2SB_HIDE, 0);

    sbreg = pci_ind(P2SB, PCI_CONFIG_BAR0) & 0xFFFFFFF0;

    pci_outb(P2SB, P2SB_HIDE, 1);

    return (!sbreg || sbreg == 0xFFFFFFF0) ? SBREG_DEFAULT : sbreg;

}

struct gpio_pad gpio_findpad(unsigned int port, unsigned int pad, unsigned int group, unsigned int bit)
{

    struct gpio_pad gpiopad;
    unsigned long community = getsbreg() + (port << 16);
    unsigned long dw0;
    unsigned int padbar;
    unsigned int value;
    unsigned int lock;
    unsigned int owner;

    gpiopad.community = community;
    gpiopad.dw0 = 0;
    gpiopad.group = group;
    gpiopad.bit = bit;

    arch_kmap(community, community, SBREG_PORTSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    padbar = readpad(community + PADBAR);

    if (!padbar || padbar >= SBREG_PORTSIZE - (pad + 1) * PADSIZE)
        return gpiopad;

    dw0 = community + padbar + pad * PADSIZE;
    value = readpad(dw0);
    lock = readpad(community + PADCFGLOCK + group * 8);
    owner = readpad(community + HOSTSWOWN + group * 4);

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "gpio: port 0x%H2u pad %u dw0 0x%H8u lock 0x%H8u owner 0x%H8u", &port, &pad, &value, &lock, &owner);

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

    unsigned long is = pad->community + GPIIS + pad->group * 4;
    unsigned long ie = pad->community + GPIIE + pad->group * 4;
    unsigned long owner = pad->community + HOSTSWOWN + pad->group * 4;
    unsigned int dw0 = readpad(pad->dw0);
    unsigned int i;

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

    if (!apic_setlevel(IRQ, 1))
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "gpio: no apic line for the GPIO interrupt");

        return 0;

    }

    if (!countinterrupts())
        apic_setroutine(IRQ, handleirq);

    interrupts[i].pad.community = pad->community;
    interrupts[i].pad.dw0 = pad->dw0;
    interrupts[i].pad.group = pad->group;
    interrupts[i].pad.bit = pad->bit;
    interrupts[i].spurious = 0;
    interrupts[i].routine = routine;

    writepad(ie, readpad(ie) | (1 << pad->bit));
    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "gpio: interrupt on, pad dw0 0x%H8u", &dw0);

    return 1;

}

void gpio_unsetinterrupt(struct gpio_pad *pad)
{

    stopall(pad);

}

void module_unregister(void)
{

    stopall((void *)0);

}

