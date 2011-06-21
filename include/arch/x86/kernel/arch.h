#ifndef ARCH_ARCH_H
#define ARCH_ARCH_H

struct mboot_info;

extern void arch_init(struct mboot_info *header, unsigned int magic, unsigned int stack);
extern void arch_interrupts_disable();
extern void arch_interrupts_enable();
extern void arch_reboot();
extern void arch_set_stack(unsigned int address);
extern void arch_usermode(unsigned int address);

#endif

