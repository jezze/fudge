#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/bus.h>
#include "amiga.h"

#define DEVICES                         (AMIGA_SERIAL + 1)
#define ROUTINES                        14
#define SHARED                          4
#define REG_CUSTOM                      0x00DFF000
#define REG_INTENAR                     0x1C
#define REG_INTREQR                     0x1E
#define REG_INTENA                      0x9A
#define REG_INTREQ                      0x9C
#define REG_INT_SET                     0x8000
#define REG_INT_MASTER                  0x4000

static struct base_bus bus;
static void (*routines[ROUTINES][SHARED])(unsigned int irq);
static volatile unsigned short *custom = (volatile unsigned short *)REG_CUSTOM;

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

/* paula raises levels 1 to 6 for its fourteen sources; every level asks for all of them and clears what it handled */
static void handleirq(void)
{

    unsigned int pending;

    while ((pending = custom[REG_INTREQR / 2] & custom[REG_INTENAR / 2] & 0x3FFF))
    {

        unsigned int irq;

        for (irq = 0; irq < ROUTINES; irq++)
        {

            if (pending & (1U << irq))
            {

                callroutines(irq);

                custom[REG_INTREQ / 2] = 1U << irq;

            }

        }

    }

}

unsigned int amiga_setroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    if (irq >= ROUTINES || !addroutine(irq, routine))
        return 0;

    custom[REG_INTENA / 2] = REG_INT_SET | REG_INT_MASTER | (1U << irq);

    return 1;

}

unsigned int amiga_unsetroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    if (irq >= ROUTINES)
        return 0;

    removeroutine(irq, routine);

    if (!hasroutines(irq))
        custom[REG_INTENA / 2] = 1U << irq;

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

    arch_kmap(REG_CUSTOM, REG_CUSTOM, 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    for (i = 1; i <= 6; i++)
        arch_setinterrupt(i, handleirq);

    base_initbus(&bus, AMIGA_BUS, "amiga", bus_setup, bus_next);

}

void module_register(void)
{

    base_registerbus(&bus);

}

void module_unregister(void)
{

    base_unregisterbus(&bus);

}

