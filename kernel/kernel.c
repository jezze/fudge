#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/event.h>
#include <kernel/initrd.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <kernel/runtime.h>
#include <kernel/symbol.h>
#include <kernel/syscall.h>

static struct kernel_core kernelCore;

void kernel_disable_interrupts()
{

    kernelCore.arch->disable_interrupts();

}

void kernel_enable_interrupts()
{

    kernelCore.arch->enable_interrupts();

}

void kernel_reboot()
{

    kernelCore.arch->reboot();

}

void kernel_register_irq(unsigned int index, void (*handler)())
{

    kernelCore.arch->register_irq(index, handler);

}

void kernel_unregister_irq(unsigned int index)
{

    kernelCore.arch->unregister_irq(index);

}

void kernel_core_init(struct kernel_core *core, struct kernel_arch *arch)
{

    core->arch = arch;

}

static void kernel_init_userspace()
{

    unsigned int id = syscall_execute("init", 0, 0);
    struct runtime_task *task = runtime_get_task(id);

    kernelCore.arch->set_stack(kernelCore.arch->stack);
    kernelCore.arch->enter_usermode(task->registers.ip, task->registers.sp);

}

void kernel_init(struct kernel_arch *arch)
{

    log_init();

    kernel_core_init(&kernelCore, arch);
    kernelCore.arch->setup(kernelCore.arch);

    modules_init();
    event_init();
    initrd_init(kernelCore.arch->initrdc, kernelCore.arch->initrdv);
    symbol_init();
    runtime_init();

    kernel_init_userspace();

    for (;;);

}

