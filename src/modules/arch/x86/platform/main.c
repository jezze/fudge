#include <fudge.h>
#include <kernel.h>
#include <modules/base/bus.h>
#include <modules/system/system.h>
#include "platform.h"

#define REG_PIT                         0x0040
#define REG_RTC                         0x0070
#define REG_UART4                       0x02E8
#define REG_UART2                       0x02F8
#define REG_UART3                       0x03E8
#define REG_UART1                       0x03F8
#define IRQ_PIT                         0x00
#define IRQ_UART2                       0x03
#define IRQ_UART1                       0x04
#define IRQ_RTC                         0x08

struct device
{

    unsigned short base;
    unsigned short irq;

};

static struct base_bus bus;

static struct device devices[] = {
    {0},
    {REG_PIT, IRQ_PIT},
    {REG_RTC, IRQ_RTC},
    {REG_UART1, IRQ_UART1},
    {REG_UART2, IRQ_UART2},
    {REG_UART3, IRQ_UART1},
    {REG_UART4, IRQ_UART2}
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

