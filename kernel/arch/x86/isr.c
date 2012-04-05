#include <kernel/mmu.h>
#include <kernel/runtime.h>
#include <kernel/error.h>
#include <kernel/irq.h>
#include <kernel/log.h>
#include <kernel/syscall.h>
#include <kernel/arch/x86/idt.h>
#include <kernel/arch/x86/isr.h>
#include <kernel/arch/x86/io.h>

static void *routines[ISR_ROUTINE_SLOTS];

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

static void remap_irq()
{

    io_outb(0x20, 0x11);
    io_outb(0xA0, 0x11);
    io_outb(0x21, 0x20);
    io_outb(0xA1, 0x28);
    io_outb(0x21, 0x04);
    io_outb(0xA1, 0x02);
    io_outb(0x21, 0x01);
    io_outb(0xA1, 0x01);
    io_outb(0x21, 0x00);
    io_outb(0xA1, 0x00);

}

static void reset_irq(unsigned int slave)
{

    if (slave)
        io_outb(0xA0, 0x20);

    io_outb(0x20, 0x20);

}

void isr_handle_cpu(struct isr_cpu_registers *registers)
{

    void (*routine)(struct isr_cpu_registers *registers) = routines[registers->index];

    if (!routine)
    {

        log_write("ERROR!\n");
        log_write("Interrupt: 0x%x\n", registers->index);
        log_write("Error code: %d\n", registers->error);
        error_panic("UNHANDLED INTERRUPT", __FILE__, __LINE__);

    }

    routine(registers);

}

void isr_handle_irq(struct isr_irq_registers *registers)
{

    if (registers->segment.ds == 0x23)
        save_state(runtime_get_running_task(), &registers->general, &registers->interrupt);

    irq_raise(registers->index);
    reset_irq(registers->slave);

    if (registers->segment.ds == 0x23)
        load_state(runtime_get_running_task(), &registers->general, &registers->interrupt);

}

void isr_handle_syscall(struct isr_syscall_registers *registers)
{

    struct runtime_task *task = runtime_get_running_task();

    save_state(task, &registers->general, &registers->interrupt);

    registers->general.eax = syscall_raise(registers->general.eax, task, registers->interrupt.esp);

    load_state(runtime_get_running_task(), &registers->general, &registers->interrupt);

}

void isr_register_routine(unsigned int index, void (*routine)(struct isr_cpu_registers *registers))
{

    routines[index] = routine;

}

void isr_unregister_routine(unsigned int index)
{

    routines[index] = 0;

}

void isr_init()
{

    remap_irq();

    idt_set_gate(ISR_ROUTINE_DE, isr_routine00, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_DB, isr_routine01, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_NI, isr_routine02, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_BP, isr_routine03, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_OF, isr_routine04, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_BR, isr_routine05, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_UD, isr_routine06, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_NM, isr_routine07, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_DF, isr_routine08, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_CO, isr_routine09, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_TS, isr_routine0A, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_NP, isr_routine0B, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_SS, isr_routine0C, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_GP, isr_routine0D, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_PF, isr_routine0E, 0x08, 0x8E);
    idt_set_gate(0x0F, isr_routine0F, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_MF, isr_routine10, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_AC, isr_routine11, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_MC, isr_routine12, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_XM, isr_routine13, 0x08, 0x8E);
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
    idt_set_gate(ISR_ROUTINE_PIT, isr_routine20, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_KBD, isr_routine21, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_CASCADE, isr_routine22, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_COM2, isr_routine23, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_COM1, isr_routine24, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_SOUND, isr_routine25, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_SDA, isr_routine26, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_PP, isr_routine27, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_RTC, isr_routine28, 0x08, 0x8E);
    idt_set_gate(0x29, isr_routine29, 0x08, 0x8E);
    idt_set_gate(0x2A, isr_routine2A, 0x08, 0x8E);
    idt_set_gate(0x2B, isr_routine2B, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_MOUSE, isr_routine2C, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_FPU, isr_routine2D, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_ATAP, isr_routine2E, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_ATAS, isr_routine2F, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_SYSCALL, isr_routine80, 0x08, 0xEE);

}

