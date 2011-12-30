#include <kernel/event.h>
#include <kernel/irq.h>
#include <kernel/modules.h>
#include <modules/pit/pit.h>

static void handle_irq(struct modules_device *self)
{

    struct pit_device *device = (struct pit_device *)self;

    device->jiffies += 1;

    event_handle(EVENT_IRQ_PIT);

}

static struct pit_device device;

void init()
{

    pit_device_init(&device);

    irq_register_routine(0x00, &device.base, handle_irq);

    modules_register_device(&device.base);

}

void destroy()
{

    irq_unregister_routine(0x00, &device.base);

    modules_unregister_device(&device.base);

}

