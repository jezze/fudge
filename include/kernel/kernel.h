#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

struct kernel_arch
{

    void (*enable_interrupts)();
    void (*disable_interrupts)();
    void (*set_stack)(unsigned int address);
    void (*enable_usermode)(unsigned int address);
    unsigned int stackAddress;
    unsigned int *mbootAddress;
    unsigned int *initrdAddress;

};

extern void kernel_assert(unsigned int condition, char *message, char *file, unsigned int line);
extern void kernel_panic(char *message, char *file, unsigned int line);
extern void kernel_init(struct kernel_arch *arch);

#endif

