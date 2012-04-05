#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

struct kernel_arch
{

    void (*setup)(struct kernel_arch *arch);
    void (*reboot)();
    void (*halt)();
    void (*enable_interrupts)();
    void (*disable_interrupts)();
    void (*enter_usermode)(unsigned int ip, unsigned int sp);
    void *stack;
    void (*set_stack)(void *address);
    unsigned int ramdiskc;
    void **ramdiskv;

};

void kernel_disable_interrupts();
void kernel_enable_interrupts();
void kernel_reboot();
void kernel_halt();
void kernel_arch_init(struct kernel_arch *arch, void (*setup)(struct kernel_arch *arch), void (*reboot)(), void (*halt)(), void (*enable_interrupts)(), void (*disable_interrupts)(), void (*enter_usermode)(unsigned int ip, unsigned int sp), void *stack, void (*set_stack)(void *address), unsigned int ramdiskc, void **ramdiskv);
void kernel_init(struct kernel_arch *arch);

#endif

