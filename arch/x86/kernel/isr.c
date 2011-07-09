#include <kernel/kernel.h>
#include <kernel/log.h>
#include <arch/x86/kernel/idt.h>
#include <arch/x86/kernel/isr.h>

static void *isrRoutines[ISR_ROUTINES_SIZE];

void isr_register_handler(unsigned char index, void (*handler)(struct isr_registers *registers))
{

    isrRoutines[index] = handler;

}

void isr_unregister_handler(unsigned char index)
{

    isrRoutines[index] = 0;

}

void isr_handler(struct isr_registers *registers)
{

    void (*handler)(struct isr_registers *registers) = isrRoutines[registers->number];

    if (handler)
    {

        handler(registers);

    }

    else
    {

        void *args[] = {&registers->number, &registers->error};
        log_message(LOG_TYPE_ERROR, "Interrupt: 0x%x, Error code: %d", args);

        kernel_panic("UNHANDLED INTERRUPT", __FILE__, __LINE__);

    }

}

void isr_init()
{

    idt_set_gate(ISR_ROUTINE_DE, isr00, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_DB, isr01, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_NI, isr02, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_BP, isr03, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_OF, isr04, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_BR, isr05, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_UD, isr06, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_NM, isr07, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_DF, isr08, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_CO, isr09, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_TS, isr0A, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_NP, isr0B, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_SS, isr0C, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_GP, isr0D, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_PF, isr0E, 0x08, 0x8E);
    idt_set_gate(0x0F, isr0F, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_MF, isr10, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_AC, isr11, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_MC, isr12, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_XM, isr13, 0x08, 0x8E);
    idt_set_gate(0x14, isr14, 0x08, 0x8E);
    idt_set_gate(0x15, isr15, 0x08, 0x8E);
    idt_set_gate(0x16, isr16, 0x08, 0x8E);
    idt_set_gate(0x17, isr17, 0x08, 0x8E);
    idt_set_gate(0x18, isr18, 0x08, 0x8E);
    idt_set_gate(0x19, isr19, 0x08, 0x8E);
    idt_set_gate(0x1A, isr1A, 0x08, 0x8E);
    idt_set_gate(0x1B, isr1B, 0x08, 0x8E);
    idt_set_gate(0x1C, isr1C, 0x08, 0x8E);
    idt_set_gate(0x1D, isr1D, 0x08, 0x8E);
    idt_set_gate(0x1E, isr1E, 0x08, 0x8E);
    idt_set_gate(0x1F, isr1F, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_SYSCALL, isr_syscall, 0x08, 0x8E);

}

