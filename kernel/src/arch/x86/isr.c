#include <error.h>
#include <event.h>
#include <mmu.h>
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

}

static void load_ustate(struct runtime_task *task, struct isr_registers *registers)
{

    registers->interrupt.cs = gdt_get_segment(GDT_INDEX_UCODE);
    registers->interrupt.eip = task->registers.ip;
    registers->interrupt.esp = task->registers.sp;
    registers->general.ebp = task->registers.sb;

}

static void save_ustate(struct runtime_task *task, struct isr_registers *registers)
{

    task->registers.ip = registers->interrupt.eip;
    task->registers.sp = registers->interrupt.esp;
    task->registers.sb = registers->general.ebp;

}

static void handle_undefined(struct isr_registers *registers)
{

    error_register(0, registers->index);
    error_register(1, registers->extra);
    error_panic("UNHANDLED INTERRUPT", __FILE__, __LINE__);

}

unsigned int isr_handle(struct isr_registers *registers)
{

    struct runtime_task *task1;
    struct runtime_task *task2;

    task1 = runtime_schedule();

    if (task1)
        save_ustate(task1, registers);

    isr_raise(registers->index, registers);
    event_raise(registers->index);

    task2 = runtime_schedule();

    if (task2)
    {

        load_ustate(task2, registers);

        mmu_load_user_memory(task2->id);

        return gdt_get_segment(GDT_INDEX_UDATA);

    }

    load_kstate(registers);

    return gdt_get_segment(GDT_INDEX_KDATA);

}

void isr_set_routine(unsigned int index, void (*routine)(struct isr_registers *registers))
{

    if (index > ISR_TABLE_SLOTS)
        return;

    routines[index] = routine;

}

void isr_unset_routine(unsigned int index)
{

    if (index > ISR_TABLE_SLOTS)
        return;

    routines[index] = 0;

}

void isr_raise(unsigned int index, struct isr_registers *registers)
{

    if (index > ISR_TABLE_SLOTS)
        return;

    routines[index](registers);

}

void isr_setup(unsigned int cs)
{

    unsigned int i;

    for (i = 0; i < ISR_TABLE_SLOTS; i++)
        isr_set_routine(i, handle_undefined);

}

