#include <lib/types.h>
#include <lib/io.h>
#include <kernel/regs.h>
#include <kernel/irq.h>
#include <kernel/pit.h>

int32_t pit_timer = 0;

void pit_handler(registers_t *r)
{

    pit_timer++;

}

void pit_wait(int32_t ticks)
{

    int32_t eticks;

    eticks = pit_timer + ticks;

    while (pit_timer < eticks);

}

void pit_init(uint32_t freq)
{

    irq_register_handler(0, pit_handler);

    uint32_t divisor = PIT_HERTZ / freq;

    outb(0x43, 0x36);

    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

    outb(0x40, l);
    outb(0x40, h);

}

