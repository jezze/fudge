#include <kernel/vfs.h>
#include <kernel/initrd.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <kernel/runtime.h>

static struct kernel kernel;

void kernel_disable_interrupts()
{

    kernel.arch->disable_interrupts();

}

void kernel_enable_interrupts()
{

    kernel.arch->enable_interrupts();

}

void kernel_register_irq(unsigned int index, void (*handler)())
{

    kernel.arch->register_irq(index, handler);

}

void kernel_unregister_irq(unsigned int index)
{

    kernel.arch->unregister_irq(index);

}

static void kernel_init_shell()
{

    struct runtime_task *task = runtime_get_free_task();
    task->load(task, "/shell", 0, 0);

    runtime_activate(task);

    kernel.arch->enter_usermode(task->eip, task->esp);

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
    runtime_init();

    kernel.arch->set_stack(arch->stackAddress);
    kernel_init_shell();

    for (;;);

}

