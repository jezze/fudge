#ifndef ARCH_ARCH_H
#define ARCH_ARCH_H

struct arch_x86
{

    struct kernel_arch base;
    struct mboot_header *header;
    unsigned int magic;
    unsigned int stack;

};

void arch_init(struct mboot_header *header, unsigned int magic);

#endif

