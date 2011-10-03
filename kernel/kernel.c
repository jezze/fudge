#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <kernel/initrd.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/shell.h>

static struct kernel kernel;

void kernel_assert(unsigned int condition, char *message, char *file, unsigned int line)
{

    if (condition)
        return;

    kernel.arch->disable_interrupts();

    void *args[] = {message, file, &line};
    log_message(LOG_TYPE_ERROR, "ASSERTION FAIL (%s) at (%s:%d)", args);

    for (;;);

}

void kernel_panic(char *message, char *file, unsigned int line)
{

    kernel.arch->disable_interrupts();

    void *args[] = {message, file, &line};
    log_message(LOG_TYPE_ERROR, "KERNEL PANIC (%s) at (%s:%d)", args);

    for (;;);

}

void kernel_init(struct kernel_arch *arch)
{

    log_init();

    kernel.arch = arch;
    kernel.arch->setup(kernel.arch);
    kernel.arch->enable_interrupts();

    vfs_init();
    initrd_init(kernel.arch->initrdAddress);
    modules_init();

    kernel.arch->set_stack(0x00400000);
    kernel.arch->enable_usermode(shell_init);

    for (;;);

}

