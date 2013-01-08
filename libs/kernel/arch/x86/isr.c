#include <fudge/kernel.h>
#include "idt.h"
#include "isr.h"

void isr_setup(unsigned short selector)
{

    unsigned int i;

    for (i = 0; i < ISR_ROUTINE_SLOTS; i++)
        idt_set_entry(i, isr_routine, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);

    isr_disable_pic();

}

