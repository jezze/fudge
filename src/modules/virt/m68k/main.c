#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/bus.h>
#include "../virt.h"

#define DEVICES                         (VIRT_TTY + 1)
#define PICS                            6
#define ROUTINES                        (PICS * 32)
#define SHARED                          4
#define REG_PIC                         0xFF000000
#define REG_PIC_PENDING                 0x04
#define REG_PIC_DISABLE                 0x0C
#define REG_PIC_ENABLE                  0x10
#define REG_RTC                         0xFF006000
#define REG_TTY                         0xFF008000
#define REG_VIRTIO                      0xFF010000
#define IRQ_RTC                         (5 * 32)
#define IRQ_TTY                         31
#define IRQ_VIRTIO                      32

struct device
{

    unsigned long base;
    unsigned int irq;

};

static struct base_bus bus;
static void (*routines[ROUTINES][SHARED])(unsigned int irq);
static struct device devices[DEVICES] = {
    {0},
    {REG_RTC, IRQ_RTC},
    {REG_RTC + 0x1000, IRQ_RTC + 1},
    {0, 0},
    {REG_VIRTIO + 0x0000, IRQ_VIRTIO + 0},
    {REG_VIRTIO + 0x0200, IRQ_VIRTIO + 1},
    {REG_VIRTIO + 0x0400, IRQ_VIRTIO + 2},
    {REG_VIRTIO + 0x0600, IRQ_VIRTIO + 3},
    {REG_VIRTIO + 0x0800, IRQ_VIRTIO + 4},
    {REG_VIRTIO + 0x0A00, IRQ_VIRTIO + 5},
    {REG_VIRTIO + 0x0C00, IRQ_VIRTIO + 6},
    {REG_VIRTIO + 0x0E00, IRQ_VIRTIO + 7},
    {0, 0},
    {REG_TTY, IRQ_TTY}
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

static volatile unsigned int *pic(unsigned int index, unsigned int offset)
{

    return (volatile unsigned int *)(unsigned long)(REG_PIC + index * 0x1000 + offset);

}

/* goldfish pic n raises interrupt level n + 1; every level asks all of them */
static void handleirq(void)
{

    unsigned int i;

    for (i = 0; i < PICS; i++)
    {

        unsigned int pending;

        while ((pending = *pic(i, REG_PIC_PENDING)))
        {

            unsigned int line;

            for (line = 0; line < 32; line++)
            {

                if (pending & (1U << line))
                    callroutines(i * 32 + line);

            }

        }

    }

}

unsigned int virt_setroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    if (irq >= ROUTINES || !addroutine(irq, routine))
        return 0;

    *pic(irq / 32, REG_PIC_ENABLE) = 1U << (irq % 32);

    return 1;

}

unsigned int virt_unsetroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    if (irq >= ROUTINES)
        return 0;

    removeroutine(irq, routine);

    if (!hasroutines(irq))
        *pic(irq / 32, REG_PIC_DISABLE) = 1U << (irq % 32);

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

    unsigned int i;

    arch_kmap(REG_PIC, REG_PIC, PICS * 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    for (i = 1; i <= PICS; i++)
        arch_setinterrupt(i, handleirq);

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

