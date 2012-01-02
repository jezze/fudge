#include <lib/string.h>
#include <kernel/error.h>
#include <kernel/vfs.h>
#include <kernel/initrd.h>
#include <kernel/irq.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/mmu.h>
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

void kernel_halt()
{

    kernelCore.arch->halt();

}

void kernel_core_init(struct kernel_core *core, struct kernel_arch *arch)
{

    core->arch = arch;

}

static void kernel_init_userspace()
{

    unsigned int id = syscall_execute("init", 0, 0);

    if (!id)
        error_panic("Could not run init", __FILE__, __LINE__);

    struct runtime_task *task = runtime_get_task(id);

    kernelCore.arch->set_stack(kernelCore.arch->stack);
    kernelCore.arch->enter_usermode(task->registers.ip, task->registers.sp);

}

void kernel_init(struct kernel_arch *arch)
{

    log_init();

    log_write("[kernel] Fudge init\n");

    kernel_core_init(&kernelCore, arch);
    kernelCore.arch->setup(kernelCore.arch);

    if (kernelCore.arch->enable_mmu)
        kernelCore.arch->enable_mmu();

    modules_init();
    initrd_init(kernelCore.arch->initrdc, kernelCore.arch->initrdv);
    symbol_init();
    runtime_init();

    kernel_init_userspace();

}

