#include <kernel/log.h>
#include <arch/x86/kernel/idt.h>
#include <arch/x86/kernel/isr.h>

void *isrRoutines[ISR_ROUTINES_SIZE];

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

    if (isrRoutines[registers->int_no] != 0)
    {

        void (*handler)(struct isr_registers *registers) = isrRoutines[registers->int_no];

        if (handler)
            handler(registers);
    
    }

    else
    {

        void *args[] = {&registers->int_no, &registers->err_code};
        log_message(LOG_TYPE_ERROR, "Unhandled interrupt: 0x%x, Error code: %d", args);

        for(;;);

    }

}

void isr_init()
{

    idt_set_gate(ISR_ROUTINE_DIVIDEBYZERO, (unsigned int)isr00, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_DEBUG, (unsigned int)isr01, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_NONMASKABLEINTERRUPT, (unsigned int)isr02, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_BREAKPOINT, (unsigned int)isr03, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_INTODETECTED, (unsigned int)isr04, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_OUTOFBOUNDS, (unsigned int)isr05, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_INVALIDOPCODE, (unsigned int)isr06, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_NOCOPROCESSOR, (unsigned int)isr07, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_DOUBLEFAULT, (unsigned int)isr08, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_COPROCESSORSEGMENTOVERRUN, (unsigned int)isr09, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_TSS, (unsigned int)isr0A, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_NOSEGMENT, (unsigned int)isr0B, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_STACKFAULT, (unsigned int)isr0C, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_GENERALPROTECTIONFAULT, (unsigned int)isr0D, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_PAGEFAULT, (unsigned int)isr0E, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_UNKNOWNINTERRUPT, (unsigned int)isr0F, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_COPROCESSORFAULT, (unsigned int)isr10, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_ALIGNMENTCHECK, (unsigned int)isr11, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_MACHINECHECK, (unsigned int)isr12, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_RESERVED13, (unsigned int)isr13, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_RESERVED14, (unsigned int)isr14, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_RESERVED15, (unsigned int)isr15, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_RESERVED16, (unsigned int)isr16, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_RESERVED17, (unsigned int)isr17, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_RESERVED18, (unsigned int)isr18, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_RESERVED19, (unsigned int)isr19, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_RESERVED1A, (unsigned int)isr1A, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_RESERVED1B, (unsigned int)isr1B, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_RESERVED1C, (unsigned int)isr1C, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_RESERVED1D, (unsigned int)isr1D, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_RESERVED1E, (unsigned int)isr1E, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_RESERVED1F, (unsigned int)isr1F, 0x08, 0x8E);
    idt_set_gate(ISR_ROUTINE_SYSCALL, (unsigned int)isr_syscall, 0x08, 0x8E);

}

