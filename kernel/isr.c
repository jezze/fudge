#include <lib/types.h>
#include <kernel/regs.h>
#include <kernel/idt.h>
#include <kernel/screen.h>
#include <kernel/isr.h>

void *isr_routines[ISR_ROUTINES_SIZE];

void isr_register_handler(uint8_t num, void (*handler)(registers_t *r))
{

    isr_routines[num] = handler;

}

void isr_unregister_handler(uint8_t num)
{

    isr_routines[num] = 0;

}

void isr_handler(registers_t *r)
{

    if (isr_routines[r->int_no] != 0)
    {

        void (*handler)(registers_t *r) = isr_routines[r->int_no];

        if (handler)
            handler(r);
    
    }

    else
    {

        screen_puts("Unhandled interrupt: 0x");
        screen_puts_hex(r->int_no);

    }

}

void isr_init()
{

    idt_set_gate(0x00, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(0x01, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_gate(0x02, (uint32_t)isr2, 0x08, 0x8E);
    idt_set_gate(0x03, (uint32_t)isr3, 0x08, 0x8E);
    idt_set_gate(0x04, (uint32_t)isr4, 0x08, 0x8E);
    idt_set_gate(0x05, (uint32_t)isr5, 0x08, 0x8E);
    idt_set_gate(0x06, (uint32_t)isr6, 0x08, 0x8E);
    idt_set_gate(0x07, (uint32_t)isr7, 0x08, 0x8E);
    idt_set_gate(0x08, (uint32_t)isr8, 0x08, 0x8E);
    idt_set_gate(0x09, (uint32_t)isr9, 0x08, 0x8E);
    idt_set_gate(0x0A, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(0x0B, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(0x0C, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(0x0D, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(0x0E, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(0x0F, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(0x10, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(0x11, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(0x12, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(0x13, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(0x14, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(0x15, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(0x16, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(0x17, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(0x18, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(0x19, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(0x1A, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(0x1B, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(0x1C, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(0x1D, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(0x1E, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(0x1F, (uint32_t)isr31, 0x08, 0x8E);
    idt_set_gate(0x80, (uint32_t)syscall, 0x08, 0x8E);

}

