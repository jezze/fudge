#include <lib/elf.h>
#include <kernel/elf.h>
#include <kernel/vfs.h>
#include <kernel/event.h>
#include <kernel/kernel.h>
#include <kernel/runtime.h>

unsigned int syscall_attach(unsigned int index, void (*handler)())
{

    struct runtime_task *task = runtime_get_running_task();    

    if (!task)
        return 0;

    return event_register(index, task, handler);

}

unsigned int syscall_close(unsigned int fd)
{

    struct runtime_task *task = runtime_get_running_task();    

    if (!task)
        return 0;

    task->remove_descriptor(task, fd);

    event_handler(EVENT_SYSCALL_CLOSE);

    return 1;

}

unsigned int syscall_detach(unsigned int index)
{

    struct runtime_task *task = runtime_get_running_task();    

    if (!task)
        return 0;

    return event_unregister(index, task);

}

unsigned int syscall_execute(char *path, unsigned int argc, char **argv)
{

    struct vfs_node *node = vfs_find("bin", path);

    if (!(node && node->read))
        return 0;

    struct runtime_task *task = runtime_get_slot();

    if (!task)
        return 0;

    struct runtime_task *ptask = runtime_get_running_task();

    if (ptask)
        task->parentid = ptask->id;
    else
        task->parentid = 0;

    void *paddress = kernel_get_task_memory(task->id);
    unsigned int limit = 0x10000;

    node->read(node, limit, paddress);

    void *vaddress = elf_get_virtual(paddress);

    if (!vaddress)
        return 0;

    void *entry = elf_get_entry(paddress);

    if (!entry)
        return 0;

    if (!task->load(task, paddress, vaddress, limit, entry, argc, argv))
        return 0;

    kernel_map_task_memory(paddress, vaddress, limit, 0x7, 0x7);

    struct vfs_node *sin = vfs_find("dev", "stdin");
    struct vfs_node *sout = vfs_find("dev", "stdout");
    struct vfs_node *serror = vfs_find("dev", "stderr");

    task->add_descriptor(task, sin);
    task->add_descriptor(task, sout);
    task->add_descriptor(task, serror);

    runtime_activate(task);

    event_handler(EVENT_SYSCALL_EXECUTE);

    return task->id;

}

unsigned int syscall_exit()
{

    struct runtime_task *task = runtime_get_running_task();

    if (!task)
        return 0;

    task->unload(task);

    struct runtime_task *ptask = runtime_get_task(task->parentid);

    if (!ptask)
        return 0;

    runtime_activate(ptask);

    event_handler(EVENT_SYSCALL_EXIT);

    return ptask->id;

}

unsigned int syscall_load(char *path)
{

    struct vfs_node *node = vfs_find("mod", path);

    if (!node)
        return 0;

    elf_relocate(node->physical);

    void *entry = elf_get_symbol(node->physical, "init");

    if (!entry)
        return 0;

    void (*init)() = entry;
    init();

    event_handler(EVENT_SYSCALL_LOAD);

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

    event_handler(EVENT_SYSCALL_OPEN);

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

    event_handler(EVENT_SYSCALL_READ);

    return c;

}

unsigned int syscall_reboot()
{

    kernel_reboot();

    return 1;

}

unsigned int syscall_unload()
{

    event_handler(EVENT_SYSCALL_UNLOAD);

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

    event_handler(EVENT_SYSCALL_WAIT);

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

    event_handler(EVENT_SYSCALL_WRITE);

    return c;

}

