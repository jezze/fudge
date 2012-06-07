#include <lib/memory.h>
#include <kernel/error.h>
#include <kernel/kernel.h>
#include <kernel/modules.h>
#include <kernel/ramdisk.h>
#include <kernel/runtime.h>
#include <kernel/syscall.h>

static struct kernel_arch *kernelArch;

struct kernel_context *kernel_get_context()
{

    return &kernelArch->context;

}

void kernel_disable_interrupts()
{

    kernelArch->disable_interrupts();

}

void kernel_enable_interrupts()
{

    kernelArch->enable_interrupts();

}

void kernel_halt()
{

    kernelArch->halt();

}

void kernel_arch_init(struct kernel_arch *arch, void (*setup)(struct kernel_arch *arch), void (*halt)(), void (*enable_interrupts)(), void (*disable_interrupts)(), void (*enter_usermode)(unsigned int ip, unsigned int sp), unsigned int ramdiskc, void **ramdiskv)
{

    memory_clear(arch, sizeof (struct kernel_arch));

    arch->setup = setup;
    arch->halt = halt;
    arch->enable_interrupts = enable_interrupts;
    arch->disable_interrupts = disable_interrupts;
    arch->enter_usermode = enter_usermode;
    arch->ramdiskc = ramdiskc;
    arch->ramdiskv = ramdiskv;

}

static struct runtime_task *execute_init()
{

    unsigned int id;
    unsigned int index;
    unsigned int slot = runtime_get_task_slot();
    kernelArch->context.running = runtime_get_task(slot);

    runtime_task_init(kernelArch->context.running, slot);
    id = syscall_open(&kernelArch->context, 0, "/ramdisk/bin/init");
    index = syscall_execute(&kernelArch->context, id);

    if (!index)
        return 0;

    return runtime_get_task(index);

}

void kernel_init(struct kernel_arch *arch)
{

    struct runtime_task *task;

    error_assert(arch != 0, "Architecture not found", __FILE__, __LINE__);

    kernelArch = arch;
    kernelArch->setup(kernelArch);

    modules_init();
    runtime_init();
    ramdisk_init(kernelArch->ramdiskc, kernelArch->ramdiskv);

    task = execute_init();

    error_assert(task != 0, "Init not found", __FILE__, __LINE__);

    kernelArch->enter_usermode(task->registers.ip, task->registers.sp);

}

