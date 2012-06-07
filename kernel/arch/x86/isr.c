#include <kernel/error.h>
#include <kernel/event.h>
#include <kernel/mmu.h>
#include <kernel/runtime.h>
#include <kernel/arch/x86/idt.h>
#include <kernel/arch/x86/isr.h>

static void (*routines[ISR_TABLE_SLOTS])(struct runtime_task *task, struct isr_cpu_registers *registers);

static void save_state(struct runtime_task *task, struct isr_general_registers *general, struct isr_interrupt_registers *interrupt)
{

    task->registers.ip = interrupt->eip;
    task->registers.sp = interrupt->esp;
    task->registers.sb = general->ebp;

}

static void load_state(struct runtime_task *task, struct isr_general_registers *general, struct isr_interrupt_registers *interrupt)
{

    interrupt->eip = task->registers.ip;
    interrupt->esp = task->registers.sp;
    general->ebp = task->registers.sb;

}

void isr_handle_cpu(struct isr_cpu_registers *registers)
{

    struct runtime_task *task;
    void (*routine)(struct runtime_task *task, struct isr_cpu_registers *registers) = routines[registers->index];

    if (!routine)
    {

        error_register(0, registers->index);
        error_register(1, registers->error);
        error_panic("UNHANDLED INTERRUPT", __FILE__, __LINE__);

    }

    task = runtime_get_running_task();

    save_state(task, &registers->general, &registers->interrupt);

    routine(task, registers);

    task = runtime_get_running_task();

    event_raise(registers->index, task);

    task = runtime_get_running_task();

    mmu_load_memory(task->id);

    load_state(task, &registers->general, &registers->interrupt);

}

void isr_register_routine(unsigned int index, void (*routine)(struct runtime_task *task, struct isr_cpu_registers *registers))
{

    routines[index] = routine;

}

void isr_unregister_routine(unsigned int index)
{

    routines[index] = 0;

}

void isr_init()
{

    idt_set_gate(ISR_INDEX_DE, isr_routine00, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_DB, isr_routine01, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_NI, isr_routine02, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_BP, isr_routine03, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_OF, isr_routine04, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_BR, isr_routine05, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_UD, isr_routine06, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_NM, isr_routine07, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_DF, isr_routine08, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_CO, isr_routine09, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_TS, isr_routine0A, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_NP, isr_routine0B, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_SS, isr_routine0C, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_GP, isr_routine0D, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_PF, isr_routine0E, 0x08, 0x8E);
    idt_set_gate(0x0F, isr_routine0F, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_MF, isr_routine10, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_AC, isr_routine11, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_MC, isr_routine12, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_XM, isr_routine13, 0x08, 0x8E);
    idt_set_gate(0x14, isr_routine14, 0x08, 0x8E);
    idt_set_gate(0x15, isr_routine15, 0x08, 0x8E);
    idt_set_gate(0x16, isr_routine16, 0x08, 0x8E);
    idt_set_gate(0x17, isr_routine17, 0x08, 0x8E);
    idt_set_gate(0x18, isr_routine18, 0x08, 0x8E);
    idt_set_gate(0x19, isr_routine19, 0x08, 0x8E);
    idt_set_gate(0x1A, isr_routine1A, 0x08, 0x8E);
    idt_set_gate(0x1B, isr_routine1B, 0x08, 0x8E);
    idt_set_gate(0x1C, isr_routine1C, 0x08, 0x8E);
    idt_set_gate(0x1D, isr_routine1D, 0x08, 0x8E);
    idt_set_gate(0x1E, isr_routine1E, 0x08, 0x8E);
    idt_set_gate(0x1F, isr_routine1F, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_PIT, isr_routine20, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_KBD, isr_routine21, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_CASCADE, isr_routine22, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_COM2, isr_routine23, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_COM1, isr_routine24, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_SOUND, isr_routine25, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_SDA, isr_routine26, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_PP, isr_routine27, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_RTC, isr_routine28, 0x08, 0x8E);
    idt_set_gate(0x29, isr_routine29, 0x08, 0x8E);
    idt_set_gate(0x2A, isr_routine2A, 0x08, 0x8E);
    idt_set_gate(0x2B, isr_routine2B, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_MOUSE, isr_routine2C, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_FPU, isr_routine2D, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_ATAP, isr_routine2E, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_ATAS, isr_routine2F, 0x08, 0x8E);
    idt_set_gate(ISR_INDEX_SYSCALL, isr_routine80, 0x08, 0xEE);

}

