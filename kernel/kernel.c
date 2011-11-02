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

static struct kernel kernel;

void kernel_disable_interrupts()
{

    kernel.arch->disable_interrupts();

}

void kernel_enable_interrupts()
{

    kernel.arch->enable_interrupts();

}

void kernel_reboot()
{

    kernel.arch->reboot();

}

void kernel_register_irq(unsigned int index, void (*handler)())
{

    kernel.arch->register_irq(index, handler);

}

void kernel_unregister_irq(unsigned int index)
{

    kernel.arch->unregister_irq(index);

}

void *kernel_get_task_memory(unsigned int pid)
{

    return kernel.arch->get_task_memory(pid);

}

void kernel_load_task_memory(void *paddress)
{

    kernel.arch->load_task_memory(paddress);

}

void kernel_map_task_memory(void *paddress, void *vaddress, unsigned int size, unsigned int tflags, unsigned int pflags)
{

    kernel.arch->map_task_memory(paddress, vaddress, size, tflags, pflags);

}

void *kernel_get_symbol(char *name)
{

    return 0;

}

void kernel_init(struct kernel_arch *arch)
{

    log_init();

    kernel.arch = arch;
    kernel.arch->setup(kernel.arch);
    kernel.arch->enable_interrupts();

    event_init();
    vfs_init();
    initrd_init(kernel.arch->initrdc, kernel.arch->initrdv);
    modules_init();
    runtime_init();

    struct runtime_task *task = runtime_get_free_task();

    task->load(task, "/init", 0, 0);
    runtime_activate(task);

    kernel.arch->set_stack(arch->stack);
    kernel.arch->enter_usermode(task->eip, task->esp);

    for (;;);

}

