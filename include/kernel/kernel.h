#ifndef KERNEL_H
#define KERNEL_H

typedef struct kernel
{

    screen_t screen;

} kernel_t;

extern uint32_t heap_address;

extern void cli();
extern void sli();

#endif

