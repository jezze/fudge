#ifndef ARCH_ARCH_H
#define ARCH_ARCH_H

#define ARCH_KERNEL_BASE                0x00000000
#define ARCH_KERNEL_SIZE                0x00400000
#define ARCH_STACK_BASE                 0x00400000

void arch_setup(unsigned int ramdiskc, void **ramdiskv);

#endif

