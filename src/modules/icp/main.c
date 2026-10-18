#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/bus.h>
#include "icp.h"

#define DEVICES                         6
#define ROUTINES                        32
#define SHARED                          4
#define REG_PIC                         0x14000000
#define REG_PIC_STATUS                  0x00
#define REG_PIC_ENABLESET               0x08
#define REG_PIC_ENABLECLR               0x0C
#define REG_TIMER1                      0x13000100
#define REG_RTC                         0x15000000
#define REG_UART0                       0x16000000
#define REG_KMI0                        0x18000000
#define REG_KMI1                        0x19000000
#define IRQ_UART0                       1
#define IRQ_KMI0                        3
#define IRQ_KMI1                        4
#define IRQ_TIMER1                      6
#define IRQ_RTC                         8

struct device
{

    unsigned int base;
    unsigned int irq;

};

static struct base_bus bus;
static volatile unsigned int *pic = (volatile unsigned int *)REG_PIC;
static void (*routines[ROUTINES][SHARED])(unsigned int irq);

static struct device devices[DEVICES] = {
    {0},
    {REG_TIMER1, IRQ_TIMER1},
    {REG_RTC, IRQ_RTC},
    {REG_UART0, IRQ_UART0},
    {REG_KMI0, IRQ_KMI0},
    {REG_KMI1, IRQ_KMI1}
};

unsigned int icp_getbase(unsigned int id)
{

    return devices[id].base;

}

unsigned int icp_getirq(unsigned int id)
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

static void handleirq(void)
{

    unsigned int status = pic[REG_PIC_STATUS / 4];
    unsigned int irq;

    for (irq = 0; irq < ROUTINES; irq++)
    {

        if (status & (1 << irq))
            callroutines(irq);

    }

}

unsigned int icp_setroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    if (irq >= ROUTINES || !addroutine(irq, routine))
        return 0;

    pic[REG_PIC_ENABLESET / 4] = 1 << irq;

    return 1;

}

unsigned int icp_unsetroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    if (irq >= ROUTINES)
        return 0;

    removeroutine(irq, routine);

    if (!hasroutines(irq))
        pic[REG_PIC_ENABLECLR / 4] = 1 << irq;

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

    arch_kmap(REG_PIC, REG_PIC, 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    pic[REG_PIC_ENABLECLR / 4] = 0xFFFFFFFF;

    arch_setirq(handleirq);
    base_initbus(&bus, ICP_BUS, "icp", bus_setup, bus_next);

}

void module_register(void)
{

    base_registerbus(&bus);

}

void module_unregister(void)
{

    base_unregisterbus(&bus);

}
