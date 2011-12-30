#include <kernel/event.h>
#include <kernel/irq.h>
#include <kernel/modules.h>
#include <modules/pit/pit.h>

static struct pit_device device;

static void handle_irq()
{

    struct pit_device *pit = &device;

    pit->jiffies += 1;

    event_handle(EVENT_IRQ_PIT);

}

void init()
{

    pit_device_init(&device);

    irq_register_routine(0x00, handle_irq);

    modules_register_device(&device.base);

}

void destroy()
{

    irq_unregister_routine(0x00);

    modules_unregister_device(&device.base);

}

