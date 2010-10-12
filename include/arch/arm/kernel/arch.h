#ifndef ARCH_ARCH_H
#define ARCH_ARCH_H

extern void arch_disable_interrupts();
extern void arch_enable_interrupts();
extern void arch_reboot();
extern void arch_init();
extern void arch_init_devices();
extern void arch_init_interrupts();
extern void arch_init_syscalls();

#endif

