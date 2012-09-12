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

static void isr_handle_undefined(struct isr_registers *registers)
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
        isr_set_routine(i, isr_handle_undefined);

    idt_set_gate(ISR_INDEX_DE, isr_routine00, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(ISR_INDEX_DB, isr_routine01, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(ISR_INDEX_NI, isr_routine02, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(ISR_INDEX_BP, isr_routine03, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(ISR_INDEX_OF, isr_routine04, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(ISR_INDEX_BR, isr_routine05, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(ISR_INDEX_UD, isr_routine06, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(ISR_INDEX_NM, isr_routine07, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(ISR_INDEX_DF, isr_routine08, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(ISR_INDEX_CO, isr_routine09, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(ISR_INDEX_TS, isr_routine0A, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(ISR_INDEX_NP, isr_routine0B, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(ISR_INDEX_SS, isr_routine0C, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(ISR_INDEX_GP, isr_routine0D, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(ISR_INDEX_PF, isr_routine0E, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x0F, isr_routine0F, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(ISR_INDEX_MF, isr_routine10, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(ISR_INDEX_AC, isr_routine11, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(ISR_INDEX_MC, isr_routine12, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(ISR_INDEX_XM, isr_routine13, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x14, isr_routine14, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x15, isr_routine15, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x16, isr_routine16, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x17, isr_routine17, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x18, isr_routine18, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x19, isr_routine19, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x1A, isr_routine1A, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x1B, isr_routine1B, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x1C, isr_routine1C, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x1D, isr_routine1D, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x1E, isr_routine1E, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x1F, isr_routine1F, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x20, isr_routine20, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x21, isr_routine21, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x22, isr_routine22, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x23, isr_routine23, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x24, isr_routine24, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x25, isr_routine25, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x26, isr_routine26, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x27, isr_routine27, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x28, isr_routine28, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x29, isr_routine29, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x2A, isr_routine2A, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x2B, isr_routine2B, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x2C, isr_routine2C, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x2D, isr_routine2D, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x2E, isr_routine2E, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(0x2F, isr_routine2F, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_gate(ISR_INDEX_SYSCALL, isr_routine80, cs, IDT_FLAG_PRESENT | IDT_FLAG_RING3 | IDT_FLAG_TYPE32INT);

}

