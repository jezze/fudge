#include <lib/types.h>
#include <lib/io.h>
#include <kernel/idt.h>
#include <kernel/isr.h>
#include <kernel/irq.h>

void *irqRoutines[IRQ_ROUTINES_SIZE];

void irq_register_handler(unsigned char index, void (*handler)(isr_registers_t *registers))
{

    irqRoutines[index] = handler;

}

void irq_unregister_handler(unsigned char index)
{

    irqRoutines[index] = 0;

}

void irq_remap()
{

    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x00);
    outb(0xA1, 0x00);

}

void irq_handler(isr_registers_t *registers)
{

    void (*handler)(isr_registers_t *registers) = irqRoutines[registers->int_no - 32];

    if (handler)
        handler(registers);

    if (registers->int_no >= 40)
        outb(0xA0, 0x20);

    outb(0x20, 0x20);

}

void irq_init()
{

    irq_remap();

    idt_set_gate(0x20, (uint32_t)irq00, 0x08, 0x8E);
    idt_set_gate(0x21, (uint32_t)irq01, 0x08, 0x8E);
    idt_set_gate(0x22, (uint32_t)irq02, 0x08, 0x8E);
    idt_set_gate(0x23, (uint32_t)irq03, 0x08, 0x8E);
    idt_set_gate(0x24, (uint32_t)irq04, 0x08, 0x8E);
    idt_set_gate(0x25, (uint32_t)irq05, 0x08, 0x8E);
    idt_set_gate(0x26, (uint32_t)irq06, 0x08, 0x8E);
    idt_set_gate(0x27, (uint32_t)irq07, 0x08, 0x8E);
    idt_set_gate(0x28, (uint32_t)irq08, 0x08, 0x8E);
    idt_set_gate(0x29, (uint32_t)irq09, 0x08, 0x8E);
    idt_set_gate(0x2A, (uint32_t)irq0A, 0x08, 0x8E);
    idt_set_gate(0x2B, (uint32_t)irq0B, 0x08, 0x8E);
    idt_set_gate(0x2C, (uint32_t)irq0C, 0x08, 0x8E);
    idt_set_gate(0x2D, (uint32_t)irq0D, 0x08, 0x8E);
    idt_set_gate(0x2E, (uint32_t)irq0E, 0x08, 0x8E);
    idt_set_gate(0x2F, (uint32_t)irq0F, 0x08, 0x8E);

}

