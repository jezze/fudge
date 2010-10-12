#ifndef ARCH_ARCH_H
#define ARCH_ARCH_H

struct mboot_info;

extern void arch_disable_interrupts();
extern void arch_enable_interrupts();
extern void arch_reboot();
extern void arch_init(struct mboot_info *header, unsigned int magic);
extern void arch_init_devices();
extern void arch_init_interrupts();
extern void arch_init_syscalls();

#endif

