#include <lib/elf.h>
#include <kernel/elf.h>
#include <kernel/vfs.h>
#include <kernel/event.h>
#include <kernel/kernel.h>
#include <kernel/mmu.h>
#include <kernel/runtime.h>

unsigned int syscall_attach(struct runtime_task *task, unsigned int index, void (*routine)())
{

    return event_register_routine(index, task, routine);

}

unsigned int syscall_close(struct runtime_task *task, unsigned int index)
{

    struct runtime_descriptor *descriptor = task->get_descriptor(task, index);

    if (!descriptor || !descriptor->node)
        return 0;

    if (descriptor->node->close)
        descriptor->node->close(descriptor->node);

    runtime_descriptor_init(descriptor, 0, 0);

    event_raise(EVENT_SYSCALL_CLOSE);

    return 1;

}

unsigned int syscall_detach(struct runtime_task *task, unsigned int index)
{

    return event_unregister_routine(index, task);

}

unsigned int syscall_halt(struct runtime_task *task)
{

    kernel_enable_interrupts();
    kernel_halt();

    return 1;

}

unsigned int syscall_execute(struct runtime_task *task, char *path, unsigned int argc, char **argv)
{

    unsigned int index = runtime_get_task_slot();

    if (!index)
        return 0;

    struct runtime_task *ntask = runtime_get_task(index);

    runtime_task_init(ntask, index);

    struct vfs_node *node = vfs_find("bin", path);

    if (!(node && node->read))
        return 0;

    node->read(node, ntask->memory.size, ntask->memory.paddress);

    ntask->memory.vaddress = elf_get_virtual(ntask->memory.paddress);

    if (!ntask->memory.vaddress)
        return 0;

    void *entry = elf_get_entry(ntask->memory.paddress);

    if (!entry)
        return 0;

    if (!ntask->load(ntask, entry, argc, argv))
        return 0;

    runtime_activate(ntask, task);
    runtime_descriptor_init(ntask->get_descriptor(ntask, 1), vfs_find("tty", "stdin"), 0);
    runtime_descriptor_init(ntask->get_descriptor(ntask, 2), vfs_find("tty", "stdout"), 0);
    runtime_descriptor_init(ntask->get_descriptor(ntask, 3), vfs_find("tty", "stderr"), 0);

    event_raise(EVENT_SYSCALL_EXECUTE);

    return ntask->id;

}

unsigned int syscall_exit(struct runtime_task *task)
{

    struct runtime_task *ptask = runtime_get_task(task->parentid);

    if (!ptask)
        return 0;

    runtime_activate(ptask, 0);

    task->unload(task);

    event_raise(EVENT_SYSCALL_EXIT);

    return ptask->id;

}

unsigned int syscall_load(struct runtime_task *task, char *path)
{

    struct vfs_node *node = vfs_find("mod", path);

    elf_relocate(node->physical);

    void (*init)() = elf_get_symbol(node->physical, "init");

    if (!init)
        return 0;

    init();

    event_raise(EVENT_SYSCALL_LOAD);

    return 1;

}

unsigned int syscall_open(struct runtime_task *task, char *view, char *name)
{

    unsigned int index = task->get_descriptor_slot(task);

    if (!index)
        return 0;

    struct runtime_descriptor *descriptor = task->get_descriptor(task, index);

    if (!descriptor)
        return 0;

    runtime_descriptor_init(descriptor, vfs_find(view, name), 0);

    if (!descriptor->node)
        return 0;

    if (descriptor->node->open)
        descriptor->node->open(descriptor->node);

    event_raise(EVENT_SYSCALL_OPEN);

    return index;

}

unsigned int syscall_read(struct runtime_task *task, unsigned int index, unsigned int count, char *buffer)
{

    struct vfs_node *node = task->get_descriptor(task, index)->node;

    if (!(node && node->read))
        return 0;

    unsigned int c = node->read(node, count, buffer);

    event_raise(EVENT_SYSCALL_READ);

    return c;

}

unsigned int syscall_reboot(struct runtime_task *task)
{

    kernel_reboot();

    return 1;

}

unsigned int syscall_unload(struct runtime_task *task, char *path)
{

    struct vfs_node *node = vfs_find("mod", path);

    if (!node)
        return 0;

    void (*destroy)() = elf_get_symbol(node->physical, "destroy");

    if (!destroy)
        return 0;

    destroy();

    event_raise(EVENT_SYSCALL_UNLOAD);

    return 1;

}

unsigned int syscall_wait(struct runtime_task *task)
{

    task->event = 0;

    struct runtime_task *ptask = runtime_get_task(task->parentid);

    if (!ptask)
        return 0;

    runtime_activate(ptask, 0);

    event_raise(EVENT_SYSCALL_WAIT);

    return ptask->id;

}

unsigned int syscall_write(struct runtime_task *task, unsigned int index, unsigned int count, char *buffer)
{

    struct vfs_node *node = task->get_descriptor(task, index)->node;

    if (!(node && node->write))
        return 0;

    unsigned int c = node->write(node, count, buffer);

    event_raise(EVENT_SYSCALL_WRITE);

    return c;

}

