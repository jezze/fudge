#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

extern void arch_init();
extern void arch_init_devices();
extern void arch_init_interrupts();
extern void arch_init_syscalls();
extern void arch_enable_interrupts();

#endif

