#include <lib/memory.h>
#include <kernel/event.h>
#include <kernel/irq.h>
#include <kernel/modules.h>
#include <modules/pit/pit.h>

static void handle_irq(struct modules_device *self)
{

    struct pit_device *device = (struct pit_device *)self;

    device->jiffies += 1;

}

static void attach(struct modules_device *device)
{

    struct pit_device *pitDevice = (struct pit_device *)device;

    irq_register_routine(pitDevice->irq, device, handle_irq);

}

static unsigned int check(struct modules_driver *self, struct modules_device *device)
{

    return device->type == PIT_DEVICE_TYPE;

}

void pit_driver_init(struct pit_driver *driver)
{

    memory_clear(driver, sizeof (struct pit_driver));

    modules_driver_init(&driver->base, PIT_DRIVER_TYPE, "pit", 0, check, attach);

}

