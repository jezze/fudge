#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

struct kernel_arch
{

    void (*setup)(struct kernel_arch *arch);
    void (*setup_mmu)();
    void (*enable_interrupts)();
    void (*disable_interrupts)();
    void (*set_stack)(void *address);
    void (*enter_usermode)(unsigned int ip, unsigned int sp);
    void (*reboot)();
    void (*halt)();
    void *stack;
    void *mboot;
    unsigned int ramdiskc;
    void **ramdiskv;

};

void kernel_disable_interrupts();
void kernel_enable_interrupts();
void kernel_reboot();
void kernel_halt();
void kernel_init(struct kernel_arch *arch);

#endif

