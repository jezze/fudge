#include <memory.h>
#include <error.h>
#include <runtime.h>
#include <arch/x86/cpu.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/isr.h>

static void (*routines[ISR_TABLE_SLOTS])(struct isr_registers *registers);

static void load_kstate(struct isr_registers *registers)
{

    registers->interrupt.cs = gdt_get_segment(GDT_INDEX_KCODE);
    registers->interrupt.eip = (unsigned int)cpu_halt;
    registers->interrupt.esp = 0x00400000;
    registers->general.ebp = 0;
    registers->general.eax = 0;

}

static void load_ustate(struct runtime_task *task, struct isr_registers *registers)
{

    registers->interrupt.cs = gdt_get_segment(GDT_INDEX_UCODE);
    registers->interrupt.eip = task->registers.ip;
    registers->interrupt.esp = task->registers.sp;
    registers->general.ebp = task->registers.sb;
    registers->general.eax = task->registers.status;

}

unsigned int isr_raise(struct isr_registers *registers)
{

    struct runtime_task *task = runtime_get_task();

    runtime_init_registers(&task->registers, registers->interrupt.eip, registers->interrupt.esp, registers->general.ebp, registers->general.eax);

    task->notify_pre_event(task, registers->index);
    routines[registers->index](registers);
    task->notify_post_event(task, registers->index);

    task = runtime_get_task();

    if (task->status.used && !task->status.idle)
    {

        load_ustate(task, registers);

        return gdt_get_segment(GDT_INDEX_UDATA);

    }

    load_kstate(registers);

    return gdt_get_segment(GDT_INDEX_KDATA);

}

void isr_set_routine(unsigned int index, void (*routine)(struct isr_registers *registers))
{

    if (index >= ISR_TABLE_SLOTS)
        return;

    routines[index] = routine;

}

void isr_unset_routine(unsigned int index)
{

    if (index >= ISR_TABLE_SLOTS)
        return;

    routines[index] = 0;

}

void isr_setup(unsigned int cs)
{

    unsigned int i;

    for (i = 0; i < ISR_TABLE_SLOTS; i++)
        idt_set_entry(i, isr_undefined, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);

}

