#include <lib/write.h>
#include <arch/x86/kernel/arch.h>
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

        write_string("Unhandled interrupt: 0x");
        write_string_hex(registers->int_no);

    }

}

void isr_init()
{

    idt_set_gate(0x00, (unsigned int)isr00, 0x08, 0x8E);
    idt_set_gate(0x01, (unsigned int)isr01, 0x08, 0x8E);
    idt_set_gate(0x02, (unsigned int)isr02, 0x08, 0x8E);
    idt_set_gate(0x03, (unsigned int)isr03, 0x08, 0x8E);
    idt_set_gate(0x04, (unsigned int)isr04, 0x08, 0x8E);
    idt_set_gate(0x05, (unsigned int)isr05, 0x08, 0x8E);
    idt_set_gate(0x06, (unsigned int)isr06, 0x08, 0x8E);
    idt_set_gate(0x07, (unsigned int)isr07, 0x08, 0x8E);
    idt_set_gate(0x08, (unsigned int)isr08, 0x08, 0x8E);
    idt_set_gate(0x09, (unsigned int)isr09, 0x08, 0x8E);
    idt_set_gate(0x0A, (unsigned int)isr0A, 0x08, 0x8E);
    idt_set_gate(0x0B, (unsigned int)isr0B, 0x08, 0x8E);
    idt_set_gate(0x0C, (unsigned int)isr0C, 0x08, 0x8E);
    idt_set_gate(0x0D, (unsigned int)isr0D, 0x08, 0x8E);
    idt_set_gate(0x0E, (unsigned int)isr0E, 0x08, 0x8E);
    idt_set_gate(0x0F, (unsigned int)isr0F, 0x08, 0x8E);
    idt_set_gate(0x10, (unsigned int)isr10, 0x08, 0x8E);
    idt_set_gate(0x11, (unsigned int)isr11, 0x08, 0x8E);
    idt_set_gate(0x12, (unsigned int)isr12, 0x08, 0x8E);
    idt_set_gate(0x13, (unsigned int)isr13, 0x08, 0x8E);
    idt_set_gate(0x14, (unsigned int)isr14, 0x08, 0x8E);
    idt_set_gate(0x15, (unsigned int)isr15, 0x08, 0x8E);
    idt_set_gate(0x16, (unsigned int)isr16, 0x08, 0x8E);
    idt_set_gate(0x17, (unsigned int)isr17, 0x08, 0x8E);
    idt_set_gate(0x18, (unsigned int)isr18, 0x08, 0x8E);
    idt_set_gate(0x19, (unsigned int)isr19, 0x08, 0x8E);
    idt_set_gate(0x1A, (unsigned int)isr1A, 0x08, 0x8E);
    idt_set_gate(0x1B, (unsigned int)isr1B, 0x08, 0x8E);
    idt_set_gate(0x1C, (unsigned int)isr1C, 0x08, 0x8E);
    idt_set_gate(0x1D, (unsigned int)isr1D, 0x08, 0x8E);
    idt_set_gate(0x1E, (unsigned int)isr1E, 0x08, 0x8E);
    idt_set_gate(0x1F, (unsigned int)isr1F, 0x08, 0x8E);
    idt_set_gate(0x80, (unsigned int)isr_syscall, 0x08, 0x8E);

}

