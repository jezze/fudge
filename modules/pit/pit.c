#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <kernel/kernel.h>
#include <modules/io/io.h>
#include <modules/pit/pit.h>

static void pit_handler()
{

}

void pit_wait(int ticks)
{

}

void pit_init()
{

    unsigned short divisor = PIT_HERTZ / PIT_FREQUENCY;

    io_outb(0x43, 0x36);
    io_outb(0x40, (unsigned char)(divisor & 0xFF));
    io_outb(0x40, (unsigned char)((divisor >> 8) & 0xFF));

    kernel_register_irq(0x00, pit_handler);

}

