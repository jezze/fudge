#include <runtime.h>
#include <syscall.h>
#include <arch/x86/arch.h>
#include <arch/x86/cpu.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/syscall.h>

static struct runtime_task *running;

unsigned short syscall_interrupt(struct syscall_registers *registers)
{

    runtime_init_registers(&running->registers, registers->interrupt.eip, registers->interrupt.esp, registers->general.ebp, registers->general.eax);
    running->registers.status = syscall_raise(registers->general.eax, running, (void *)registers->interrupt.esp);
    running = running->notify_interrupt(running, registers->general.eax);

    if (running->status.used && !running->status.idle)
    {

        registers->interrupt.cs = gdt_get_selector(GDT_INDEX_UCODE);
        registers->interrupt.eip = running->registers.ip;
        registers->interrupt.esp = running->registers.sp;
        registers->general.ebp = running->registers.fp;
        registers->general.eax = running->registers.status;

        return gdt_get_selector(GDT_INDEX_UDATA);

    }

    registers->interrupt.cs = gdt_get_selector(GDT_INDEX_KCODE);
    registers->interrupt.eip = (unsigned int)cpu_halt;
    registers->interrupt.esp = ARCH_STACK_BASE;
    registers->general.ebp = 0;
    registers->general.eax = 0;

    return gdt_get_selector(GDT_INDEX_KDATA);

}

void syscall_setup_arch(struct runtime_task *task, unsigned short selector)
{

    idt_set_entry(0x80, syscall_routine, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING3 | IDT_FLAG_TYPE32INT);

    running = task;

}

