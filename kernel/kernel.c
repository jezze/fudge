#include <kernel/dev.h>
#include <kernel/initrd.h>
#include <kernel/kernel.h>
#include <kernel/shell.h>
#include <kernel/vfs.h>

struct initrd_header *kernelInitrd;
unsigned int kernelStack;

void kernel_set_initrd(unsigned int *initrd)
{

    kernelInitrd = (struct initrd_header *)initrd;

}

void kernel_init_usermode()
{

    shell_init();

}

void kernel_init(unsigned int stack)
{

    kernelStack = stack;

    vfs_init();
    initrd_init(kernelInitrd);
    dev_init();

    kernel_init_usermode();

    for (;;);

}

