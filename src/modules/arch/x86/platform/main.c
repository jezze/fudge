#include <fudge.h>
#include <kernel.h>
#include <modules/base/bus.h>
#include <modules/system/system.h>
#include "platform.h"

#define R_PIT                           0x0040
#define R_RTC                           0x0070
#define R_UART4                         0x02E8
#define R_UART2                         0x02F8
#define R_UART3                         0x03E8
#define R_UART1                         0x03F8
#define I_PIT                           0x00
#define I_UART2                         0x03
#define I_UART1                         0x04
#define I_RTC                           0x08

struct device
{

    unsigned short base;
    unsigned short irq;

};

static struct base_bus bus;

static struct device devices[] = {
    {0},
    {R_PIT, I_PIT},
    {R_RTC, I_RTC},
    {R_UART1, I_UART1},
    {R_UART2, I_UART2},
    {R_UART3, I_UART1},
    {R_UART4, I_UART2}
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

