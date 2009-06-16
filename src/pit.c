#include <system.h>
#include <irq.h>

int32_t timer_ticks = 0;

void pit_handler(registers_t *r)
{

	timer_ticks++;

}

void pit_wait(int32_t ticks)
{

	int32_t eticks;

	eticks = timer_ticks + ticks;

	while (timer_ticks < eticks);

}

void pit_init(uint32_t frequency)
{

	irq_register_handler(0, pit_handler);

	uint32_t divisor = 1193180 / frequency;

	outb(0x43, 0x36);

	uint8_t l = (uint8_t)(divisor & 0xFF);
	uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

	outb(0x40, l);
	outb(0x40, h);

}

