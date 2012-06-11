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

void load_usermode()
{

    unsigned int id;
    unsigned int slot = runtime_get_task_slot();
    kernelArch->context.running = runtime_get_task(slot);

    runtime_task_init(kernelArch->context.running, slot);
    id = syscall_open(&kernelArch->context, 0, "/ramdisk/bin/init");

    error_assert(id != 0, "Init not found", __FILE__, __LINE__);

    slot = syscall_execute(&kernelArch->context, id);

    kernelArch->context.running = runtime_get_task(slot);
    kernelArch->enter_usermode(kernelArch->context.running->registers.ip, kernelArch->context.running->registers.sp);

}

void kernel_init(struct kernel_arch *arch)
{

    memory_copy(0xB8000, "A ", 2);

    error_assert(arch != 0, "Architecture not found", __FILE__, __LINE__);

    memory_copy(0xB8000, "B ", 2);

    kernelArch = arch;
    kernelArch->setup(kernelArch);

    modules_init();
    runtime_init();
    ramdisk_init(kernelArch->ramdiskc, kernelArch->ramdiskv);
    load_usermode();

}

