#include <lib/elf.h>
#include <lib/file.h>
#include <kernel/vfs.h>
#include <kernel/event.h>
#include <kernel/initrd.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <kernel/runtime.h>
#include <modules/elf/elf.h>

static struct kernel_core core;

void kernel_disable_interrupts()
{

    core.arch->disable_interrupts();

}

void kernel_enable_interrupts()
{

    core.arch->enable_interrupts();

}

void kernel_reboot()
{

    core.arch->reboot();

}

void kernel_register_irq(unsigned int index, void (*handler)())
{

    core.arch->register_irq(index, handler);

}

void kernel_unregister_irq(unsigned int index)
{

    core.arch->unregister_irq(index);

}

void *kernel_get_task_memory(unsigned int pid)
{

    return core.arch->get_task_memory(pid);

}

void kernel_load_task_memory(void *paddress)
{

    core.arch->load_task_memory(paddress);

}

void kernel_map_task_memory(void *paddress, void *vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    core.arch->map_task_memory(paddress, vaddress, size, tflags, pflags);

}

void *kernel_get_symbol(char *name)
{

    return 0;

}

extern void kernel_core_init(struct kernel_core *core, struct kernel_arch *arch)
{

    core->arch = arch;

}

void kernel_init(struct kernel_arch *arch)
{

    log_init();

    kernel_core_init(&core, arch);
    core.arch->setup(core.arch);

    event_init();
    vfs_init();
    initrd_init(core.arch->initrdc, core.arch->initrdv);
    modules_init();
    runtime_init();

    struct runtime_task *task = runtime_get_free_task();

    task->load(task, "/init", 0, 0);
    runtime_activate(task);

    core.arch->set_stack(core.arch->stack);
    core.arch->enter_usermode(task->registers.ip, task->registers.sp);

    for (;;);

}

