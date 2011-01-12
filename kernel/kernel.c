#include <kernel/dev.h>
#include <kernel/initrd.h>
#include <kernel/kernel.h>
#include <kernel/shell.h>
#include <kernel/tty.h>
#include <kernel/vfs.h>

unsigned int kernelInitrdAddress;

void kernel_set_initrd(unsigned int initrdAddress)
{

    kernelInitrdAddress = initrdAddress;

}

void kernel_init()
{

    vfs_init();
    initrd_init(kernelInitrdAddress);
    dev_init();
    tty_init();
    shell_init();

    for (;;);

}

