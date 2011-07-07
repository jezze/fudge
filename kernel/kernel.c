#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <kernel/initrd.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/shell.h>

static struct kernel_arch *kernelArch;

void kernel_assert(unsigned int condition, char *message, char *file, unsigned int line)
{

    if (condition)
        return;

    kernelArch->disable_interrupts();

    void *args[] = {message, file, &line};
    log_message(LOG_TYPE_ERROR, "ASSERTION FAIL (%s) at (%s:%d)", args);

    for (;;);

}

void kernel_panic(char *message, char *file, unsigned int line)
{

    kernelArch->disable_interrupts();

    void *args[] = {message, file, &line};
    log_message(LOG_TYPE_ERROR, "KERNEL PANIC (%s) at (%s:%d)", args);

    for (;;);

}

void kernel_init(struct kernel_arch *arch)
{

    kernelArch = arch;

    vfs_init();
    initrd_init(kernelArch->initrdAddress);
    modules_init();

    kernelArch->set_stack(0x00400000);
    kernelArch->enable_usermode((unsigned int)shell_init);

    for (;;);

}

