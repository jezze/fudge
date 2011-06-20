#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

extern void kernel_assert(unsigned int condition, char *message, char *file, unsigned int line);
extern void kernel_panic(char *message, char *file, unsigned int line);
extern void kernel_set_initrd(unsigned int *initrdAddress);
extern void kernel_init(unsigned int stackAddress);

#endif

