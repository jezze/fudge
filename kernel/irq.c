#include <kernel/event.h>
#include <kernel/irq.h>
#include <kernel/modules.h>
#include <kernel/runtime.h>

static struct irq_routine routines[IRQ_ROUTINE_SLOTS];

unsigned int irq_register_routine(unsigned int index, struct modules_device *device, void (*callback)(struct modules_device *device))
{

    struct irq_routine *routine = &routines[index];

    if (routine->device)
        return 0;

    routine->device = device;
    routine->callback = callback;

    return 1;

}

unsigned int irq_unregister_routine(unsigned int index, struct modules_device *device)
{

    struct irq_routine *routine = &routines[index];

    if (routine->device != device)
        return 0;

    routine->device = 0;
    routine->callback = 0;

    return 1;

}

void irq_raise(unsigned int index, struct runtime_task *task)
{

    struct irq_routine *routine = &routines[index];

    if (!routine->callback)
        return;

    routine->callback(routine->device);

    event_raise(index, task);

}

