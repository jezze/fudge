#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

struct kernel_context
{

    struct runtime_task *running;

};

struct kernel_arch
{

    void (*setup)(struct kernel_arch *arch);
    void (*halt)();
    void (*enable_interrupts)();
    void (*disable_interrupts)();
    void (*enter_usermode)(unsigned int ip, unsigned int sp);
    unsigned int ramdiskc;
    void **ramdiskv;
    struct kernel_context context;

};

struct runtime_task *kernel_get_running_task();
void kernel_set_running_task(struct runtime_task *task);
void kernel_disable_interrupts();
void kernel_enable_interrupts();
void kernel_halt();
void kernel_arch_init(struct kernel_arch *arch, void (*setup)(struct kernel_arch *arch), void (*halt)(), void (*enable_interrupts)(), void (*disable_interrupts)(), void (*enter_usermode)(unsigned int ip, unsigned int sp), unsigned int ramdiskc, void **ramdiskv);
void kernel_init(struct kernel_arch *arch);

#endif

