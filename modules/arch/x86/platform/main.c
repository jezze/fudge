#include <module.h>
#include <kernel/resource.h>
#include <base/base.h>
#include "platform.h"

enum platform_register
{

    PLATFORM_REGISTER_PIT               = 0x0040,
    PLATFORM_REGISTER_RTC               = 0x0070,
    PLATFORM_REGISTER_UART1             = 0x03F8,
    PLATFORM_REGISTER_UART2             = 0x02F8,
    PLATFORM_REGISTER_UART3             = 0x03E8,
    PLATFORM_REGISTER_UART4             = 0x02E8

};

enum platform_irq
{

    PLATFORM_IRQ_PIT                    = 0x00,
    PLATFORM_IRQ_RTC                    = 0x08,
    PLATFORM_IRQ_UART1                  = 0x04,
    PLATFORM_IRQ_UART2                  = 0x03

};

static struct base_bus bus;

unsigned short platform_bus_get_base(struct base_bus *bus, unsigned int type)
{

    switch (type)
    {

        case PLATFORM_PIT_DEVICE_TYPE:
            return PLATFORM_REGISTER_PIT;

        case PLATFORM_RTC_DEVICE_TYPE:
            return PLATFORM_REGISTER_RTC;

        case PLATFORM_UART1_DEVICE_TYPE:
            return PLATFORM_REGISTER_UART1;

        case PLATFORM_UART2_DEVICE_TYPE:
            return PLATFORM_REGISTER_UART2;

        case PLATFORM_UART3_DEVICE_TYPE:
            return PLATFORM_REGISTER_UART3;

        case PLATFORM_UART4_DEVICE_TYPE:
            return PLATFORM_REGISTER_UART4;

    }

    return 0xFFFF;

}

static void setup(struct base_bus *self)
{

}

static unsigned int device_next(struct base_bus *self, unsigned int id)
{

    switch (id)
    {

        case 0:
            return PLATFORM_PIT_DEVICE_TYPE;

        case PLATFORM_PIT_DEVICE_TYPE:
            return PLATFORM_RTC_DEVICE_TYPE;

        case PLATFORM_RTC_DEVICE_TYPE:
            return PLATFORM_UART1_DEVICE_TYPE;

        case PLATFORM_UART1_DEVICE_TYPE:
            return PLATFORM_UART2_DEVICE_TYPE;

        case PLATFORM_UART2_DEVICE_TYPE:
            return PLATFORM_UART3_DEVICE_TYPE;

        case PLATFORM_UART3_DEVICE_TYPE:
            return PLATFORM_UART4_DEVICE_TYPE;

    }

    return 0;

}

static unsigned short device_irq(struct base_bus *self, unsigned int id)
{

    switch (id)
    {

        case PLATFORM_PIT_DEVICE_TYPE:
            return PLATFORM_IRQ_PIT;

        case PLATFORM_RTC_DEVICE_TYPE:
            return PLATFORM_IRQ_RTC;

        case PLATFORM_UART1_DEVICE_TYPE:
            return PLATFORM_IRQ_UART1;

        case PLATFORM_UART2_DEVICE_TYPE:
            return PLATFORM_IRQ_UART2;

        case PLATFORM_UART3_DEVICE_TYPE:
            return PLATFORM_IRQ_UART1;

        case PLATFORM_UART4_DEVICE_TYPE:
            return PLATFORM_IRQ_UART2;

    }

    return 0xFFFF;

}

void init()
{

    base_init_bus(&bus, PLATFORM_BUS_TYPE, "platform", setup, device_next, device_irq);
    base_register_bus(&bus);

}

void destroy()
{

    base_unregister_bus(&bus);

}

