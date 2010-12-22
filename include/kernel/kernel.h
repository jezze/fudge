#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

extern void arch_interrupts_disable();

extern void kernel_assert(unsigned int condition, char *message, char *file, unsigned int line);
extern void kernel_panic(char *message, char *file, unsigned int line);
extern void kernel_init();

#endif

