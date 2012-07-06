#include <memory.h>
#include <error.h>
#include <kernel.h>
#include <modules.h>
#include <ramdisk.h>
#include <runtime.h>
#include <syscall.h>

static void load_usermode(struct kernel_arch *arch)
{

    unsigned int id;
    unsigned int slot;

    runtime_task_init(arch->running, 1);

    id = syscall_open(arch->running, 0, "/ramdisk/bin/init");

    error_assert(id != 0, "Init not found", __FILE__, __LINE__);

    slot = syscall_execute(arch->running, id);

    arch->running = runtime_get_task(slot);
    arch->running->parent = 0;
    arch->running->wait = 0;
    arch->enter_usermode(arch->running->registers.ip, arch->running->registers.sp);

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

