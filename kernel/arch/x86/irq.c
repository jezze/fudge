#include <lib/io.h>
#include <kernel/irq.h>
#include <kernel/kernel.h>
#include <kernel/runtime.h>
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

static void handle_interrupt(struct kernel_context *context, struct isr_cpu_registers *registers)
{

    irq_raise(registers->index - 0x20);
    reset(registers->error);

}

void irq_init()
{

    remap();

    isr_register_routine(ISR_INDEX_PIT, handle_interrupt);
    isr_register_routine(ISR_INDEX_KBD, handle_interrupt);
    isr_register_routine(ISR_INDEX_CASCADE, handle_interrupt);
    isr_register_routine(ISR_INDEX_COM2, handle_interrupt);
    isr_register_routine(ISR_INDEX_COM1, handle_interrupt);
    isr_register_routine(ISR_INDEX_SOUND, handle_interrupt);
    isr_register_routine(ISR_INDEX_SDA, handle_interrupt);
    isr_register_routine(ISR_INDEX_PP, handle_interrupt);
    isr_register_routine(ISR_INDEX_RTC, handle_interrupt);
    isr_register_routine(ISR_INDEX_MOUSE, handle_interrupt);
    isr_register_routine(ISR_INDEX_FPU, handle_interrupt);
    isr_register_routine(ISR_INDEX_ATAP, handle_interrupt);
    isr_register_routine(ISR_INDEX_ATAS, handle_interrupt);

}

