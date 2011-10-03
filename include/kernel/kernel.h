#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

struct kernel_arch
{

    void (*setup)(struct kernel_arch *arch);
    void (*enable_interrupts)();
    void (*disable_interrupts)();
    void (*set_stack)(unsigned int address);
    void (*enable_usermode)(void *callback);
    unsigned int stackAddress;
    unsigned int *mbootAddress;
    unsigned int *initrdAddress;

};

struct kernel
{

    struct kernel_arch *arch;

};

extern void kernel_assert(unsigned int condition, char *message, char *file, unsigned int line);
extern void kernel_panic(char *message, char *file, unsigned int line);
extern void kernel_init(struct kernel_arch *arch);

#endif

