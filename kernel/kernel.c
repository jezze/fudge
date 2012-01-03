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

static struct kernel_arch *primary;

void kernel_disable_interrupts()
{

    primary->disable_interrupts();

}

void kernel_enable_interrupts()
{

    primary->enable_interrupts();

}

void kernel_reboot()
{

    primary->reboot();

}

void kernel_halt()
{

    primary->halt();

}

void kernel_register_arch(struct kernel_arch *arch)
{

    primary = arch;

}

void kernel_init()
{

    log_init();
    log_write("[kernel] Initializing\n");

    if (!primary)
        error_panic("No architecture registered", __FILE__, __LINE__);

    primary->setup(primary);

    if (primary->setup_mmu)
        primary->setup_mmu();

    modules_init();
    runtime_init();
    initrd_init(primary->initrdc, primary->initrdv);
    symbol_init();

    unsigned int id = syscall_execute(0, "init", 0, 0);

    if (!id)
        error_panic("Could not start init", __FILE__, __LINE__);

    struct runtime_task *task = runtime_get_task(id);

    primary->set_stack(primary->stack);
    primary->enter_usermode(task->registers.ip, task->registers.sp);

}

