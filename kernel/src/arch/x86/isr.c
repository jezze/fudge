#include <memory.h>
#include <error.h>
#include <runtime.h>
#include <arch/x86/arch.h>
#include <arch/x86/cpu.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/isr.h>

static struct runtime_task *running;
static void (*routines[ISR_ROUTINE_SLOTS])(struct isr_registers *registers);

struct runtime_task *isr_get_task()
{

    return running;

}

unsigned short isr_raise(struct isr_registers *registers)
{

    runtime_init_registers(&running->registers, registers->interrupt.eip, registers->interrupt.esp, registers->general.ebp, registers->general.eax);

    running->notify_interrupt(running, registers->index);

    routines[registers->index](registers);

    running->notify_complete(running);

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

void isr_set_routine(unsigned int index, void (*routine)(struct isr_registers *registers))
{

    if (index >= ISR_ROUTINE_SLOTS)
        return;

    routines[index] = routine;

}

void isr_set_task(struct runtime_task *task)
{

    running = task;

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

