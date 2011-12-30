#include <kernel/irq.h>
#include <kernel/modules.h>

static void *routines[IRQ_ROUTINE_SLOTS];

void irq_register_routine(unsigned char index, void (*routine)())
{

    routines[index] = routine;

}

void irq_unregister_routine(unsigned char index)
{

    routines[index] = 0;

}

void irq_routine(unsigned char index)
{

    void (*routine)() = routines[index];

    if (routine)
        routine();

}

