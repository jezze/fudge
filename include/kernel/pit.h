#ifndef PIT_H
#define PIT_H

#define PIT_HERTZ 1193180
#define PIT_FREQUENCY 1000

extern int32_t pit_timer;

extern void pit_handler(registers_t *r);
extern void pit_wait(int32_t ticks);
extern void pit_init(uint32_t freq);

#endif

