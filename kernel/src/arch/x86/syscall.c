#include <runtime.h>
#include <syscall.h>
#include <arch/x86/isr.h>
#include <arch/x86/syscall.h>

static void handle_interrupt(struct isr_registers *registers)
{

    registers->general.eax = syscall_raise(registers->extra, (void *)registers->interrupt.esp);

}

void syscall_setup_arch()
{

    isr_register_routine(ISR_INDEX_SYSCALL, handle_interrupt);

}

