#include <lib/memory.h>
#include <kernel/error.h>
#include <kernel/kernel.h>
#include <kernel/modules.h>
#include <kernel/ramdisk.h>
#include <kernel/runtime.h>
#include <kernel/syscall.h>

static void load_usermode(struct kernel_arch *arch)
{

    unsigned int id;
    unsigned int slot = runtime_get_task_slot();
    arch->context.running = runtime_get_task(slot);

    runtime_task_init(arch->context.running, slot);
    id = syscall_open(&arch->context, 0, "/ramdisk/bin/init");

    error_assert(id != 0, "Init not found", __FILE__, __LINE__);

    slot = syscall_execute(&arch->context, id);

    arch->context.running = runtime_get_task(slot);
    arch->enter_usermode(arch->context.running->registers.ip, arch->context.running->registers.sp);

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

void kernel_init(struct kernel_arch *arch)
{

    error_assert(arch != 0, "Architecture not found", __FILE__, __LINE__);

    arch->setup(arch);

    modules_init();
    runtime_init();
    ramdisk_init(arch->ramdiskc, arch->ramdiskv);
    load_usermode(arch);

}

