#include <kernel/modules.h>
#include <kernel/irq.h>

static struct irq_routine routines[IRQ_ROUTINE_SLOTS];

void irq_register_routine(unsigned char index, struct modules_device *device, void (*callback)(struct modules_device *device))
{

    struct irq_routine *routine = &routines[index];

    if (routine->callback)
        return;

    routine->device = device;
    routine->callback = callback;

}

void irq_unregister_routine(unsigned char index, struct modules_device *device)
{

    struct irq_routine *routine = &routines[index];

    if (routine->device != device)
        return;

    routine->device = 0;
    routine->callback = 0;

}

void irq_routine(unsigned char index)
{

    struct irq_routine *routine = &routines[index];

    if (!routine->callback)
        return;

    routine->callback(routine->device);

}

