#ifndef ARCH_ARCH_H
#define ARCH_ARCH_H

struct arch_interface
{

    struct kernel_interface base;

};

void arch_init_interface(struct arch_interface *interface, unsigned int ramdiskc, void **ramdiskv);

#endif

