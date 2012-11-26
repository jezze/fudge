#include <memory.h>
#include <error.h>
#include <arch/x86/idt.h>
#include <arch/x86/isr.h>

void isr_setup(unsigned short selector)
{

    unsigned int i;

    for (i = 0; i < ISR_ROUTINE_SLOTS; i++)
        idt_set_entry(i, isr_routine, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);

}

