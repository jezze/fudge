#ifndef ARCH_PIT_H
#define ARCH_PIT_H

#define PIT_HERTZ     1193180
#define PIT_FREQUENCY 1000

extern void pit_wait(int ticks);
extern void pit_init();

#endif

