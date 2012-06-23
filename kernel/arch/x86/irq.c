#include <lib/io.h>
#include <kernel/arch/x86/irq.h>

void irq_init()
{

    io_outb(0xA1, 0xFF);
    io_outb(0x21, 0xFF);

}

