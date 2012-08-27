#ifndef ARCH_ARCH_H
#define ARCH_ARCH_H

struct arch_interface
{

    struct kernel_interface base;
    struct mboot_header *header;
    unsigned int magic;

};

void arch_interface_init(struct arch_interface *interface, struct mboot_header *header, unsigned int magic);
void arch_setup(struct mboot_header *header, unsigned int magic);

#endif

