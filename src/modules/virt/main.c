#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/bus.h>
#include "virt.h"

#define DEVICES                         (VIRT_VIRTIO + VIRT_VIRTIOS)
#define ROUTINES                        64
#define SHARED                          4
#define REG_PLIC                        0x0C000000
#define REG_PLIC_PRIORITY               0x000000
#define REG_PLIC_ENABLE                 0x002080
#define REG_PLIC_THRESHOLD              0x201000
#define REG_PLIC_CLAIM                  0x201004
#define REG_RTC                         0x00101000
#define REG_UART0                       0x10000000
#define REG_VIRTIO                      0x10001000
#define IRQ_RTC                         11
#define IRQ_UART0                       10
#define IRQ_VIRTIO                      1

struct device
{

    unsigned long base;
    unsigned int irq;

};

static struct base_bus bus;
static void (*routines[ROUTINES][SHARED])(unsigned int irq);

static struct device devices[DEVICES] = {
    {0},
    {0, 0},
    {REG_RTC, IRQ_RTC},
    {REG_UART0, IRQ_UART0},
    {REG_VIRTIO + 0x0000, IRQ_VIRTIO + 0},
    {REG_VIRTIO + 0x1000, IRQ_VIRTIO + 1},
    {REG_VIRTIO + 0x2000, IRQ_VIRTIO + 2},
    {REG_VIRTIO + 0x3000, IRQ_VIRTIO + 3},
    {REG_VIRTIO + 0x4000, IRQ_VIRTIO + 4},
    {REG_VIRTIO + 0x5000, IRQ_VIRTIO + 5},
    {REG_VIRTIO + 0x6000, IRQ_VIRTIO + 6},
    {REG_VIRTIO + 0x7000, IRQ_VIRTIO + 7}
};

unsigned long virt_getbase(unsigned int id)
{

    return devices[id].base;

}

unsigned int virt_getirq(unsigned int id)
{

    return devices[id].irq;

}

static unsigned int hasroutines(unsigned int irq)
{

    unsigned int i;

    for (i = 0; i < SHARED; i++)
    {

        if (routines[irq][i])
            return 1;

    }

    return 0;

}

static void callroutines(unsigned int irq)
{

    unsigned int i;

    for (i = 0; i < SHARED; i++)
    {

        if (routines[irq][i])
            routines[irq][i](irq);

    }

}

static unsigned int addroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    unsigned int i;

    for (i = 0; i < SHARED; i++)
    {

        if (routines[irq][i] == routine)
            return 1;

    }

    for (i = 0; i < SHARED; i++)
    {

        if (!routines[irq][i])
        {

            routines[irq][i] = routine;

            return 1;

        }

    }

    return 0;

}

static void removeroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    unsigned int i;

    for (i = 0; i < SHARED; i++)
    {

        if (routines[irq][i] == routine)
            routines[irq][i] = 0;

    }

}

static volatile unsigned int *plic(unsigned int offset)
{

    return (volatile unsigned int *)(unsigned long)(REG_PLIC + offset);

}

static void setenable(unsigned int irq, unsigned int enable)
{

    volatile unsigned int *word = plic(REG_PLIC_ENABLE + (irq / 32) * 4);

    if (enable)
        *word |= 1 << (irq % 32);
    else
        *word &= ~(1 << (irq % 32));

}

/* hart 0's supervisor context: claim each pending source, run its routines and complete it */
static void handleirq(void)
{

    unsigned int irq;

    while ((irq = *plic(REG_PLIC_CLAIM)))
    {

        if (irq < ROUTINES)
            callroutines(irq);

        *plic(REG_PLIC_CLAIM) = irq;

    }

}

unsigned int virt_setroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    if (irq >= ROUTINES || !addroutine(irq, routine))
        return 0;

    *plic(REG_PLIC_PRIORITY + irq * 4) = 1;
    setenable(irq, 1);

    return 1;

}

unsigned int virt_unsetroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    if (irq >= ROUTINES)
        return 0;

    removeroutine(irq, routine);

    if (!hasroutines(irq))
        setenable(irq, 0);

    return 1;

}

static void bus_setup(void)
{

}

static unsigned int bus_next(unsigned int id)
{

    return (id + 1) % DEVICES;

}

void module_init(void)
{

    arch_kmap(REG_PLIC + REG_PLIC_PRIORITY, REG_PLIC + REG_PLIC_PRIORITY, 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);
    arch_kmap(REG_PLIC + (REG_PLIC_ENABLE & ~0xFFF), REG_PLIC + (REG_PLIC_ENABLE & ~0xFFF), 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);
    arch_kmap(REG_PLIC + REG_PLIC_THRESHOLD, REG_PLIC + REG_PLIC_THRESHOLD, 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    *plic(REG_PLIC_THRESHOLD) = 0;

    arch_setinterrupt(CPU_CAUSE_EXTERNAL, handleirq);
    base_initbus(&bus, VIRT_BUS, "virt", bus_setup, bus_next);

}

void module_register(void)
{

    base_registerbus(&bus);

}

void module_unregister(void)
{

    base_unregisterbus(&bus);

}
