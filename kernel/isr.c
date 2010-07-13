#include <lib/types.h>
#include <kernel/idt.h>
#include <kernel/screen.h>
#include <kernel/isr.h>

void *isrRoutines[ISR_ROUTINES_SIZE];

void isr_register_handler(unsigned char index, void (*handler)(isr_registers_t *registers))
{

    isrRoutines[index] = handler;

}

void isr_unregister_handler(unsigned char index)
{

    isrRoutines[index] = 0;

}

void isr_handler(isr_registers_t *registers)
{

    if (isrRoutines[registers->int_no] != 0)
    {

        void (*handler)(isr_registers_t *registers) = isrRoutines[registers->int_no];

        if (handler)
            handler(registers);
    
    }

    else
    {

        screen_puts("Unhandled interrupt: 0x");
        screen_puts_hex(registers->int_no);

    }

}

void isr_init()
{

    idt_set_gate(0x00, (uint32_t)isr00, 0x08, 0x8E);
    idt_set_gate(0x01, (uint32_t)isr01, 0x08, 0x8E);
    idt_set_gate(0x02, (uint32_t)isr02, 0x08, 0x8E);
    idt_set_gate(0x03, (uint32_t)isr03, 0x08, 0x8E);
    idt_set_gate(0x04, (uint32_t)isr04, 0x08, 0x8E);
    idt_set_gate(0x05, (uint32_t)isr05, 0x08, 0x8E);
    idt_set_gate(0x06, (uint32_t)isr06, 0x08, 0x8E);
    idt_set_gate(0x07, (uint32_t)isr07, 0x08, 0x8E);
    idt_set_gate(0x08, (uint32_t)isr08, 0x08, 0x8E);
    idt_set_gate(0x09, (uint32_t)isr09, 0x08, 0x8E);
    idt_set_gate(0x0A, (uint32_t)isr0A, 0x08, 0x8E);
    idt_set_gate(0x0B, (uint32_t)isr0B, 0x08, 0x8E);
    idt_set_gate(0x0C, (uint32_t)isr0C, 0x08, 0x8E);
    idt_set_gate(0x0D, (uint32_t)isr0D, 0x08, 0x8E);
    idt_set_gate(0x0E, (uint32_t)isr0E, 0x08, 0x8E);
    idt_set_gate(0x0F, (uint32_t)isr0F, 0x08, 0x8E);
    idt_set_gate(0x10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(0x11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(0x12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(0x13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(0x14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(0x15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(0x16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(0x17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(0x18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(0x19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(0x1A, (uint32_t)isr1A, 0x08, 0x8E);
    idt_set_gate(0x1B, (uint32_t)isr1B, 0x08, 0x8E);
    idt_set_gate(0x1C, (uint32_t)isr1C, 0x08, 0x8E);
    idt_set_gate(0x1D, (uint32_t)isr1D, 0x08, 0x8E);
    idt_set_gate(0x1E, (uint32_t)isr1E, 0x08, 0x8E);
    idt_set_gate(0x1F, (uint32_t)isr1F, 0x08, 0x8E);

}

