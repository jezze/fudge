#include <lib/memory.h>
#include <kernel/irq.h>
#include <kernel/modules.h>

static struct irq_routine routines[IRQ_TABLE_SLOTS];

unsigned int irq_register_routine(unsigned int index, struct modules_device *device, void (*callback)(struct modules_device *device))
{

    if (routines[index].device)
        return 0;

    irq_routine_init(&routines[index], device, callback);

    return 1;

}

unsigned int irq_unregister_routine(unsigned int index, struct modules_device *device)
{

    if (routines[index].device != device)
        return 0;

    irq_routine_init(&routines[index], 0, 0);

    return 1;

}

void irq_raise(unsigned int index)
{

    if (!routines[index].callback)
        return;

    routines[index].callback(routines[index].device);

}

void irq_routine_init(struct irq_routine *routine, struct modules_device *device, void (*callback)(struct modules_device *device))
{

    memory_clear(routine, sizeof (struct irq_routine));

    routine->device = device;
    routine->callback = callback;

}

