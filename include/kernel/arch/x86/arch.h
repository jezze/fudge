#ifndef ARCH_ARCH_H
#define ARCH_ARCH_H

struct mboot_header;

extern void arch_init(struct mboot_header *header, unsigned int magic, void *stack);
extern void arch_reboot();

#endif

