#ifndef KERNEL_H
#define KERNEL_H

typedef struct kernel
{

    screen_t screen;

} kernel_t;

extern kernel_t kernel;

extern void cli();
extern void sli();

#endif

