#include <lib/file.h>
#include <kernel/initrd.h>
#include <kernel/kernel.h>
#include <kernel/modules.h>
#include <kernel/shell.h>
#include <kernel/vfs.h>
#include <arch/x86/kernel/cpu.h>
#include <arch/x86/kernel/tss.h>

unsigned int kernelInitrdAddress;
unsigned int kernelStackAddress;

void kernel_set_initrd(unsigned int address)
{

    kernelInitrdAddress = address;

}

void kernel_init_usermode()
{

    shell_init();

}

void kernel_init(unsigned int stackAddress)
{

    kernelStackAddress = stackAddress;

    vfs_init();
    initrd_init(kernelInitrdAddress);
    modules_init();

    unsigned int address = cpu_get_stack();
    tss_set_stack(stackAddress);

//    cpu_usermode();

    kernel_init_usermode();

    for (;;);

}

