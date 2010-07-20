#include <kernel/arch/i386/io.h>
#include <kernel/arch/i386/isr.h>
#include <kernel/arch/i386/irq.h>
#include <kernel/arch/i386/pit.h>

int pitTimer = 0;

void pit_handler(struct isr_registers *registers)
{

    pitTimer++;

}

void pit_wait(int ticks)
{

    int eticks;

    eticks = pitTimer + ticks;

    while (pitTimer < eticks);

}

void pit_init()
{

    unsigned short divisor = PIT_HERTZ / PIT_FREQUENCY;

    outb(0x43, 0x36);
    outb(0x40, (unsigned char)(divisor & 0xFF));
    outb(0x40, (unsigned char)((divisor >> 8) & 0xFF));

}

