#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

struct kernel_arch
{

    void (*setup)(struct kernel_arch *arch);
    void (*enable_interrupts)();
    void (*disable_interrupts)();
    void (*set_stack)(void *address);
    void (*enter_usermode)(unsigned int ip, unsigned int sp);
    void (*reboot)();
    void (*halt)();
    void *stack;
    void *mboot;
    unsigned int initrdc;
    void **initrdv;

};

struct kernel_core
{

    struct kernel_arch *arch;

};

extern void kernel_disable_interrupts();
extern void kernel_enable_interrupts();
extern void kernel_reboot();
extern void kernel_halt();
extern void kernel_core_init(struct kernel_core *core, struct kernel_arch *arch);
extern void kernel_init(struct kernel_arch *arch);

#endif

