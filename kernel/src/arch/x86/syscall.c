#include <runtime.h>
#include <syscall.h>
#include <arch/x86/arch.h>
#include <arch/x86/cpu.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/isr.h>
#include <arch/x86/syscall.h>

static struct runtime_task *running;

static void handle_interrupt(struct isr_registers *registers)
{

    running->registers.status = syscall_raise(registers->extra, running, (void *)registers->interrupt.esp);

}

void syscall_begin(struct isr_registers *registers)
{

    runtime_init_registers(&running->registers, registers->interrupt.eip, registers->interrupt.esp, registers->general.ebp, registers->general.eax);

}

unsigned short syscall_complete(struct isr_registers *registers)
{

    running = running->notify_interrupt(running, registers->index);

    if (running->status.used && !running->status.idle)
    {

        registers->interrupt.cs = gdt_get_selector(GDT_INDEX_UCODE);
        registers->interrupt.eip = running->registers.ip;
        registers->interrupt.esp = running->registers.sp;
        registers->general.ebp = running->registers.sb;
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
    isr_set_routine(0x80, handle_interrupt);

    running = task;

}

