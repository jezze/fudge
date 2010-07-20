#ifndef PIT_H
#define PIT_H

#define PIT_HERTZ 1193180
#define PIT_FREQUENCY 1000

struct isr_registers;

extern void pit_handler(struct isr_registers *registers);
extern void pit_wait(int ticks);
extern void pit_init();

#endif

