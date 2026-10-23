#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/bus.h>
#include "amiga.h"

#define DEVICES                         (AMIGA_VIDEO + 1)
#define ROUTINES                        14
#define SHARED                          4
#define REG_CUSTOM                      0x00DFF000
#define REG_INTENAR                     0x1C
#define REG_INTREQR                     0x1E
#define REG_INTENA                      0x9A
#define REG_INTREQ                      0x9C
#define REG_INT_SET                     0x8000
#define REG_INT_MASTER                  0x4000
#define CIAROUTINES                     5
#define REG_CIAA                        0x00BFE000
#define REG_CIAA_ICR                    0xD01
#define REG_CIAA_ICR_SET                0x80
#define REG_CIAA_ICR_ALL                0x7F

static struct base_bus bus;
static void (*routines[ROUTINES][SHARED])(unsigned int irq);
static void (*ciaroutines[CIAROUTINES])(unsigned int irq);
static volatile unsigned short *custom = (volatile unsigned short *)REG_CUSTOM;
static volatile unsigned char *ciaa = (volatile unsigned char *)REG_CIAA;

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

/* reading cia-a's icr clears all its flags, so one read serves every source on it */
static void handleports(unsigned int irq)
{

    unsigned int icr = ciaa[REG_CIAA_ICR];
    unsigned int i;

    for (i = 0; i < CIAROUTINES; i++)
    {

        if ((icr & (1U << i)) && ciaroutines[i])
            ciaroutines[i](i);

    }

}

unsigned int amiga_setciaroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    if (irq >= CIAROUTINES || ciaroutines[irq])
        return 0;

    ciaroutines[irq] = routine;
    ciaa[REG_CIAA_ICR] = REG_CIAA_ICR_SET | (1U << irq);

    return amiga_setroutine(AMIGA_IRQ_PORTS, handleports);

}

unsigned int amiga_unsetciaroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    unsigned int i;

    if (irq >= CIAROUTINES || ciaroutines[irq] != routine)
        return 0;

    ciaa[REG_CIAA_ICR] = 1U << irq;
    ciaroutines[irq] = 0;

    for (i = 0; i < CIAROUTINES; i++)
    {

        if (ciaroutines[i])
            return 1;

    }

    return amiga_unsetroutine(AMIGA_IRQ_PORTS, handleports);

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
    arch_kmap(REG_CIAA, REG_CIAA, 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    ciaa[REG_CIAA_ICR] = REG_CIAA_ICR_ALL;

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

