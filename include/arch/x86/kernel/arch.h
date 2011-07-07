#ifndef ARCH_ARCH_H
#define ARCH_ARCH_H

struct mboot_info;

extern void arch_init(struct mboot_info *header, unsigned int magic, unsigned int stack);
extern void arch_reboot();

#endif

