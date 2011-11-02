#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

struct kernel_arch
{

    void (*setup)(struct kernel_arch *arch);
    void (*enable_interrupts)();
    void (*disable_interrupts)();
    void (*set_stack)(void *address);
    void (*enter_usermode)(void *callback, void *stack);
    void (*register_irq)(unsigned char index, void (*handler)());
    void (*unregister_irq)(unsigned char index);
    void *(*get_task_memory)(unsigned int pid);
    void (*load_task_memory)(void *paddress);
    void (*map_task_memory)(void *paddress, void *vaddress, unsigned int size, unsigned int tflags, unsigned int pflags);
    void (*reboot)();
    void *stack;
    void *mboot;
    unsigned int initrdc;
    void **initrdv;

};

struct kernel
{

    struct kernel_arch *arch;

};

extern void kernel_disable_interrupts();
extern void kernel_enable_interrupts();
extern void kernel_reboot();
extern void kernel_register_irq(unsigned int index, void (*handler)());
extern void kernel_unregister_irq(unsigned int index);
extern void *kernel_get_task_memory(unsigned int pid);
extern void kernel_load_task_memory(void *paddress);
extern void kernel_map_task_memory(void *paddress, void *vaddress, unsigned int size, unsigned int tflags, unsigned int pflags);
extern void kernel_init(struct kernel_arch *arch);

#endif

