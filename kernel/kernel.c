#include <lib/file.h>
#include <kernel/modules.h>
#include <kernel/initrd.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/shell.h>
#include <kernel/vfs.h>
#include <arch/x86/kernel/arch.h>
#include <arch/x86/kernel/cpu.h>
#include <arch/x86/kernel/tss.h>

unsigned int *kernelInitrdAddress;
unsigned int kernelStackAddress;

void kernel_assert(unsigned int condition, char *message, char *file, unsigned int line)
{

    if (condition)
        return;

    arch_interrupts_disable();

    void *args[] = {message, file, &line};
    log_message(LOG_TYPE_ERROR, "ASSERTION FAIL (%s) at (%s:%d)", args);

    for (;;);

}

void kernel_panic(char *message, char *file, unsigned int line)
{

    arch_interrupts_disable();

    void *args[] = {message, file, &line};
    log_message(LOG_TYPE_ERROR, "KERNEL PANIC (%s) at (%s:%d)", args);

    for (;;);

}

void kernel_set_initrd(unsigned int *address)
{

    kernelInitrdAddress = address;

}

void kernel_init(unsigned int stackAddress)
{

    kernelStackAddress = stackAddress;

    vfs_init();
    initrd_init(kernelInitrdAddress);
    modules_init();

    tss_set_stack(0x00400000);
    cpu_usermode((unsigned int)shell_init);

    for (;;);

}

