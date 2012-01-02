#include <kernel/error.h>
#include <kernel/vfs.h>
#include <kernel/initrd.h>
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

void kernel_init(struct kernel_arch *arch)
{

    struct kernel_core *core = &kernelCore;

    log_init();
    log_write("[kernel] Fudge init\n");

    kernel_core_init(core, arch);
    core->arch->setup(core->arch);

    if (core->arch->enable_mmu)
        core->arch->enable_mmu();

    modules_init();
    runtime_init();
    initrd_init(core->arch->initrdc, core->arch->initrdv);
    symbol_init();

    unsigned int id = syscall_execute("init", 0, 0);

    if (!id)
        error_panic("Could not run init", __FILE__, __LINE__);

    struct runtime_task *task = runtime_get_task(id);

    core->arch->set_stack(core->arch->stack);
    core->arch->enter_usermode(task->registers.ip, task->registers.sp);

}

