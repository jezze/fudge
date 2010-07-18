#include <kernel/arch/i386/gdt.h>
#include <kernel/arch/i386/idt.h>

void arch_init()
{

    gdt_init();
    idt_init();

}

