#include <module.h>
#include <kernel/resource.h>
#include <base/base.h>
#include "platform.h"

static void scan(struct base_bus *self)
{

}

static unsigned short device_irq(struct base_bus *self, struct base_device *device)
{

    switch (device->type)
    {

        case PLATFORM_PIT_DEVICE_TYPE:
            return PLATFORM_PIT_IRQ;

        case PLATFORM_RTC_DEVICE_TYPE:
            return PLATFORM_RTC_IRQ;

        case PLATFORM_UART1_DEVICE_TYPE:
            return PLATFORM_UART1_IRQ;

        case PLATFORM_UART2_DEVICE_TYPE:
            return PLATFORM_UART2_IRQ;

    }

    return 0xFFFF;

}

void platform_init_bus(struct platform_bus *bus)
{

    memory_clear(bus, sizeof (struct platform_bus));
    base_init_bus(&bus->base, "platform", scan, device_irq);

}

