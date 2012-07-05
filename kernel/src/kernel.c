#include <memory.h>
#include <error.h>
#include <isr.h>
#include <kernel.h>
#include <modules.h>
#include <ramdisk.h>
#include <runtime.h>
#include <syscall.h>

static void load_usermode(struct kernel_arch *arch)
{

    unsigned int id;
    unsigned int slot;

    arch->context->running = runtime_get_task(1);

    runtime_task_init(arch->context->running, 1);

    id = syscall_open(arch->context, 0, "/ramdisk/bin/init");

    error_assert(id != 0, "Init not found", __FILE__, __LINE__);

    slot = syscall_execute(arch->context, id);

    arch->context->running = runtime_get_task(slot);
    arch->context->running->parentid = 0;
    arch->enter_usermode(arch->context->running->registers.ip, arch->context->running->registers.sp);

}

static void start(struct kernel_arch *self)
{

    self->setup(self);

    modules_init();
    runtime_init();
    ramdisk_init(self->ramdiskc, self->ramdiskv);
    load_usermode(self);

}

void kernel_arch_init(struct kernel_arch *arch, void (*setup)(struct kernel_arch *self), void (*halt)(), void (*enable_interrupts)(), void (*disable_interrupts)(), void (*enter_usermode)(unsigned int ip, unsigned int sp), unsigned int ramdiskc, void **ramdiskv)
{

    memory_clear(arch, sizeof (struct kernel_arch));

    arch->setup = setup;
    arch->halt = halt;
    arch->enable_interrupts = enable_interrupts;
    arch->disable_interrupts = disable_interrupts;
    arch->enter_usermode = enter_usermode;
    arch->ramdiskc = ramdiskc;
    arch->ramdiskv = ramdiskv;
    arch->start = start;

}

