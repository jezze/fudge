#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

extern void kernel_set_initrd(unsigned int *initrd);
extern void kernel_init(unsigned int stack);

#endif

