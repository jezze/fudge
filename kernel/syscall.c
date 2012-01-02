#include <lib/elf.h>
#include <kernel/elf.h>
#include <kernel/vfs.h>
#include <kernel/event.h>
#include <kernel/kernel.h>
#include <kernel/mmu.h>
#include <kernel/runtime.h>

unsigned int syscall_attach(unsigned int index, void (*routine)())
{

    struct runtime_task *task = runtime_get_running_task();    

    if (!task)
        return 0;

    return event_register_routine(index, task, routine);

}

unsigned int syscall_close(unsigned int fd)
{

    struct runtime_task *task = runtime_get_running_task();    

    if (!task)
        return 0;

    task->remove_descriptor(task, fd);

    event_raise(EVENT_SYSCALL_CLOSE);

    return 1;

}

unsigned int syscall_detach(unsigned int index)
{

    struct runtime_task *task = runtime_get_running_task();    

    if (!task)
        return 0;

    return event_unregister_routine(index, task);

}

unsigned int syscall_halt()
{

    kernel_halt();

    return 1;

}

unsigned int syscall_execute(char *path, unsigned int argc, char **argv)
{

    struct vfs_node *node = vfs_find("bin", path);

    if (!(node && node->read))
        return 0;

    struct runtime_task *task = runtime_get_slot();

    if (!task)
        return 0;

    node->read(node, task->memory.size, task->memory.paddress);

    task->memory.vaddress = elf_get_virtual(task->memory.paddress);

    if (!task->memory.vaddress)
        return 0;

    void *entry = elf_get_entry(task->memory.paddress);

    if (!entry)
        return 0;

    if (!task->load(task, entry, argc, argv))
        return 0;

    mmu_map_user_memory(&task->memory);

    struct runtime_task *ptask = runtime_get_running_task();

    if (ptask)
        task->parentid = ptask->id;
    else
        task->parentid = 0;

    runtime_activate(task);

    struct vfs_node *sin = vfs_find("tty", "stdin");
    struct vfs_node *sout = vfs_find("tty", "stdout");
    struct vfs_node *serror = vfs_find("tty", "stderr");

    task->add_descriptor(task, sin);
    task->add_descriptor(task, sout);
    task->add_descriptor(task, serror);

    event_raise(EVENT_SYSCALL_EXECUTE);

    return task->id;

}

unsigned int syscall_exit()
{

    struct runtime_task *task = runtime_get_running_task();

    if (!task)
        return 0;

    task->unload(task);

    mmu_unmap_memory(&task->memory);

    struct runtime_task *ptask = runtime_get_task(task->parentid);

    if (!ptask)
        return 0;

    runtime_activate(ptask);

    event_raise(EVENT_SYSCALL_EXIT);

    return ptask->id;

}

unsigned int syscall_load(char *path)
{

    struct vfs_node *node = vfs_find("mod", path);

    if (!node)
        return 0;

    elf_relocate(node->physical);

    void (*init)() = elf_get_symbol(node->physical, "init");

    if (!init)
        return 0;

    init();

    event_raise(EVENT_SYSCALL_LOAD);

    return 1;

}

unsigned int syscall_open(char *view, char *name)
{

    struct runtime_task *task = runtime_get_running_task();    

    if (!task)
        return 0;

    struct vfs_node *node = vfs_find(view, name);

    if (!node)
        return 0;

    struct runtime_descriptor *descriptor = task->add_descriptor(task, node);

    if (!descriptor)
        return 0;

    event_raise(EVENT_SYSCALL_OPEN);

    return descriptor->id;

}

unsigned int syscall_read(unsigned int fd, unsigned int count, char *buffer)
{

    struct runtime_task *task = runtime_get_running_task();    

    if (!task)
        return 0;

    struct vfs_node *node = task->get_descriptor(task, fd)->node;

    if (!(node && node->read))
        return 0;

    unsigned int c = node->read(node, count, buffer);

    event_raise(EVENT_SYSCALL_READ);

    return c;

}

unsigned int syscall_reboot()
{

    kernel_reboot();

    return 1;

}

unsigned int syscall_unload(char *path)
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

unsigned int syscall_wait()
{

    struct runtime_task *oldtask = runtime_get_running_task();

    if (!oldtask)
        return 0;

    struct runtime_task *task = runtime_get_task(oldtask->parentid);

    if (!task)
        return 0;

    runtime_activate(task);

    event_raise(EVENT_SYSCALL_WAIT);

    return 1;

}

unsigned int syscall_write(unsigned int fd, unsigned int count, char *buffer)
{

    struct runtime_task *task = runtime_get_running_task();    

    if (!task)
        return 0;

    struct vfs_node *node = task->get_descriptor(task, fd)->node;

    if (!(node && node->write))
        return 0;

    unsigned int c = node->write(node, count, buffer);

    event_raise(EVENT_SYSCALL_WRITE);

    return c;

}

