#ifndef PIT_H
#define PIT_H

#define PIT_FREQUENCY 100

extern void pit_wait(int32_t);
extern void pit_init(uint32_t);

#endif

