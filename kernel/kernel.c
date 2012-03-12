#include <kernel/error.h>
#include <kernel/vfs.h>
#include <kernel/ramdisk.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/mmu.h>
#include <kernel/modules.h>
#include <kernel/runtime.h>
#include <kernel/symbol.h>
#include <kernel/syscall.h>

static struct kernel_arch *kernelArch;

void kernel_disable_interrupts()
{

    kernelArch->disable_interrupts();

}

void kernel_enable_interrupts()
{

    kernelArch->enable_interrupts();

}

void kernel_reboot()
{

    kernelArch->reboot();

}

void kernel_halt()
{

    kernelArch->halt();

}

void kernel_init(struct kernel_arch *arch)
{

    kernelArch = arch;

    log_write("[kernel] Initializing\n");

    if (!kernelArch)
        error_panic("No architecture registered", __FILE__, __LINE__);

    kernelArch->setup(kernelArch);

    if (kernelArch->setup_mmu)
        kernelArch->setup_mmu();

    vfs_init();
    syscall_init();
    runtime_init();
    ramdisk_init(kernelArch->ramdiskc, kernelArch->ramdiskv);
    symbol_init();

    unsigned int index = syscall_execute(0, "/ramdisk/bin/init", 0, 0);

    if (!index)
        error_panic("Could not start init", __FILE__, __LINE__);

    struct runtime_task *task = runtime_get_task(index);

    kernelArch->set_stack(kernelArch->stack);
    kernelArch->enter_usermode(task->registers.ip, task->registers.sp);

}

