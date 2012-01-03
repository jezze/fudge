#include <kernel/irq.h>
#include <kernel/mmu.h>
#include <kernel/runtime.h>
#include <kernel/arch/x86/idt.h>
#include <kernel/arch/x86/io.h>
#include <kernel/arch/x86/irq.h>
#include <kernel/arch/x86/isr.h>

void irq_handle(struct irq_registers *registers)
{

    struct runtime_task *task = runtime_get_running_task();

    task->registers.ip = registers->eip;
    task->registers.sp = registers->useresp;
    task->registers.sb = registers->ebp;

    irq_raise(registers->index);

    if (registers->slave)
        io_outb(0xA0, 0x20);

    io_outb(0x20, 0x20);

    runtime_get_state(&registers->eip, &registers->useresp, &registers->ebp);

}

static void irq_remap()
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

void irq_init()
{

    irq_remap();

    idt_set_gate(0x20, irq00, 0x08, 0x8E);
    idt_set_gate(0x21, irq01, 0x08, 0x8E);
    idt_set_gate(0x22, irq02, 0x08, 0x8E);
    idt_set_gate(0x23, irq03, 0x08, 0x8E);
    idt_set_gate(0x24, irq04, 0x08, 0x8E);
    idt_set_gate(0x25, irq05, 0x08, 0x8E);
    idt_set_gate(0x26, irq06, 0x08, 0x8E);
    idt_set_gate(0x27, irq07, 0x08, 0x8E);
    idt_set_gate(0x28, irq08, 0x08, 0x8E);
    idt_set_gate(0x29, irq09, 0x08, 0x8E);
    idt_set_gate(0x2A, irq0A, 0x08, 0x8E);
    idt_set_gate(0x2B, irq0B, 0x08, 0x8E);
    idt_set_gate(0x2C, irq0C, 0x08, 0x8E);
    idt_set_gate(0x2D, irq0D, 0x08, 0x8E);
    idt_set_gate(0x2E, irq0E, 0x08, 0x8E);
    idt_set_gate(0x2F, irq0F, 0x08, 0x8E);

}

