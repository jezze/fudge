#ifndef ARCH_ARCH_H
#define ARCH_ARCH_H

struct mboot_info;

extern void arch_assert(unsigned int condition, char *message, char *file, unsigned int line);
extern void arch_panic(char *message, char *file, unsigned int line);
extern void arch_init(struct mboot_info *header, unsigned int magic);
extern void arch_interrupts_disable();
extern void arch_interrupts_enable();
extern void arch_reboot();

#endif

