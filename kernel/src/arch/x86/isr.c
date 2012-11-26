#include <memory.h>
#include <error.h>
#include <runtime.h>
#include <arch/x86/idt.h>
#include <arch/x86/isr.h>
#include <arch/x86/syscall.h>

static void (*routines[ISR_ROUTINE_SLOTS])(struct isr_registers *registers);

void isr_raise(struct isr_registers *registers)
{

    routines[registers->index](registers);

}

void isr_set_routine(unsigned int index, void (*routine)(struct isr_registers *registers))
{

    if (index >= ISR_ROUTINE_SLOTS)
        return;

    routines[index] = routine;

}

void isr_unset_routine(unsigned int index)
{

    if (index >= ISR_ROUTINE_SLOTS)
        return;

    routines[index] = 0;

}

void isr_setup(unsigned short selector)
{

    unsigned int i;

    for (i = 0; i < ISR_ROUTINE_SLOTS; i++)
        idt_set_entry(i, isr_undefined, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);

}

