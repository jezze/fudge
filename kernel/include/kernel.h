#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

struct kernel_arch
{

    void (*start)(struct kernel_arch *self);
    void (*setup)(struct kernel_arch *self);
    void (*enter_usermode)(unsigned int ip, unsigned int sp);
    unsigned int ramdiskc;
    void **ramdiskv;

};

void kernel_arch_init(struct kernel_arch *arch, void (*setup)(struct kernel_arch *self), void (*enter_usermode)(unsigned int ip, unsigned int sp), unsigned int ramdiskc, void **ramdiskv);

#endif

