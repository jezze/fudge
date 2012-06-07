#include <lib/io.h>
#include <kernel/irq.h>
#include <kernel/arch/x86/isr.h>
#include <kernel/arch/x86/irq.h>

static void remap()
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

static void reset(unsigned int slave)
{

    if (slave)
        io_outb(0xA0, 0x20);

    io_outb(0x20, 0x20);

}

static void handle_irq(struct isr_cpu_registers *registers)
{

    irq_raise(registers->index - 0x20);
    reset(registers->error);

}

void irq_init()
{

    remap();

    isr_register_routine(0x20, handle_irq);
    isr_register_routine(0x21, handle_irq);
    isr_register_routine(0x22, handle_irq);
    isr_register_routine(0x23, handle_irq);
    isr_register_routine(0x24, handle_irq);
    isr_register_routine(0x25, handle_irq);
    isr_register_routine(0x26, handle_irq);
    isr_register_routine(0x27, handle_irq);
    isr_register_routine(0x28, handle_irq);
    isr_register_routine(0x29, handle_irq);
    isr_register_routine(0x2A, handle_irq);
    isr_register_routine(0x2B, handle_irq);
    isr_register_routine(0x2C, handle_irq);
    isr_register_routine(0x2D, handle_irq);
    isr_register_routine(0x2E, handle_irq);
    isr_register_routine(0x2F, handle_irq);

}

