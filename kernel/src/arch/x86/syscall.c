#include <runtime.h>
#include <syscall.h>
#include <arch/x86/idt.h>
#include <arch/x86/isr.h>
#include <arch/x86/syscall.h>

static void handle_interrupt(struct isr_registers *registers)
{

    registers->general.eax = syscall_raise(registers->extra, (void *)registers->interrupt.esp);

}

void syscall_setup_arch(unsigned int cs)
{

    idt_set_entry(0x80, syscall_routine, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING3 | IDT_FLAG_TYPE32INT);
    isr_set_routine(0x80, handle_interrupt);

}

