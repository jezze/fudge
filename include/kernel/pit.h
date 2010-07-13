#ifndef PIT_H
#define PIT_H

#define PIT_HERTZ 1193180
#define PIT_FREQUENCY 1000

extern int pitTimer;

extern void pit_handler(isr_registers_t *registers);
extern void pit_wait(int ticks);
extern void pit_init();

#endif

