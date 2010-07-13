#include <lib/types.h>
#include <lib/io.h>
#include <kernel/isr.h>
#include <kernel/irq.h>
#include <kernel/pit.h>

int pitTimer = 0;

void pit_handler(isr_registers_t *registers)
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

    uint16_t divisor = PIT_HERTZ / PIT_FREQUENCY;

    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));

}

