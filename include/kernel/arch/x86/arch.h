#ifndef ARCH_ARCH_H
#define ARCH_ARCH_H

struct arch_x86
{

    struct kernel_arch base;
    struct mboot_header *header;
    unsigned int magic;
    void *stack;

};

void arch_x86_init(struct arch_x86 *x86, struct mboot_header *header, unsigned int magic, void *stack);
void arch_init(struct mboot_header *header, unsigned int magic, void *stack);

#endif

