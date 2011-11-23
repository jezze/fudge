#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/event.h>
#include <kernel/initrd.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <kernel/runtime.h>
#include <kernel/symbol.h>

static struct kernel_core kernelCore;

void kernel_disable_interrupts()
{

    kernelCore.arch->disable_interrupts();

}

void kernel_enable_interrupts()
{

    kernelCore.arch->enable_interrupts();

}

void kernel_enter_usermode(void *callback, void *stack)
{

    kernelCore.arch->enter_usermode(callback, stack);

}

void kernel_reboot()
{

    kernelCore.arch->reboot();

}

void kernel_register_irq(unsigned int index, void (*handler)())
{

    kernelCore.arch->register_irq(index, handler);

}

void kernel_unregister_irq(unsigned int index)
{

    kernelCore.arch->unregister_irq(index);

}

void *kernel_get_task_memory(unsigned int id)
{

    return kernelCore.arch->get_task_memory(id);

}

void kernel_load_task_memory(void *paddress)
{

    kernelCore.arch->load_task_memory(paddress);

}

void kernel_map_task_memory(void *paddress, void *vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    kernelCore.arch->map_task_memory(paddress, vaddress, size, tflags, pflags);

}

void kernel_core_init(struct kernel_core *core, struct kernel_arch *arch)
{

    core->arch = arch;

}

void kernel_init(struct kernel_arch *arch)
{

    log_init();

    kernel_core_init(&kernelCore, arch);
    kernelCore.arch->setup(kernelCore.arch);

    modules_init();
    event_init();
    initrd_init(kernelCore.arch->initrdc, kernelCore.arch->initrdv);
    symbol_init();
    runtime_init();

    struct runtime_task *task = runtime_get_slot();

    task->load(task, "init", 0, 0);
    runtime_activate(task);

    kernelCore.arch->set_stack(kernelCore.arch->stack);
    kernelCore.arch->enter_usermode(task->registers.ip, task->registers.sp);

    for (;;);

}

