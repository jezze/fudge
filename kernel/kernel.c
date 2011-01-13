#include <kernel/dev.h>
#include <kernel/initrd.h>
#include <kernel/kernel.h>
#include <kernel/shell.h>
#include <kernel/tty.h>
#include <kernel/vfs.h>

unsigned int *kernelInitrd;

void kernel_set_initrd(unsigned int *initrd)
{

    kernelInitrd = initrd;

}

void kernel_init()
{

    vfs_init();
    initrd_init(*kernelInitrd);
    dev_init();
    tty_init();
    shell_init();

    for (;;);

}

