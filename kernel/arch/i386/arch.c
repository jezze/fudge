#include <kernel/arch/i386/gdt.h>
#include <kernel/arch/i386/idt.h>
#include <kernel/arch/i386/arch.h>

void arch_init()
{

    gdt_init();
    idt_init();

}

