#include <fudge.h>
#include <kernel.h>
#include <modules/base/bus.h>
#include <modules/system/system.h>
#include "platform.h"

#define REGISTERPIT                     0x0040
#define REGISTERRTC                     0x0070
#define REGISTERUART4                   0x02E8
#define REGISTERUART2                   0x02F8
#define REGISTERUART3                   0x03E8
#define REGISTERUART1                   0x03F8
#define IRQPIT                          0x00
#define IRQUART2                        0x03
#define IRQUART1                        0x04
#define IRQRTC                          0x08

struct device
{

    unsigned short base;
    unsigned short irq;

};

static struct base_bus bus;

static struct device devices[] = {
    {0},
    {REGISTERPIT, IRQPIT},
    {REGISTERRTC, IRQRTC},
    {REGISTERUART1, IRQUART1},
    {REGISTERUART2, IRQUART2},
    {REGISTERUART3, IRQUART1},
    {REGISTERUART4, IRQUART2}
};

unsigned short platform_getbase(unsigned int id)
{

    return devices[id].base;

}

unsigned short platform_getirq(unsigned int id)
{

    return devices[id].irq;

}

static void bus_setup(void)
{

}

static unsigned int bus_next(unsigned int id)
{

    return (id + 1) % 7;

}

void module_init(void)
{

    base_initbus(&bus, PLATFORM_BUS, "platform", bus_setup, bus_next);

}

void module_register(void)
{

    base_registerbus(&bus);

}

void module_unregister(void)
{

    base_unregisterbus(&bus);

}

