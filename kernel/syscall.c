#include <lib/file.h>
#include <kernel/vfs.h>
#include <kernel/event.h>
#include <kernel/kernel.h>
#include <kernel/runtime.h>

unsigned int syscall_attach(unsigned int index, void (*handler)())
{

    struct runtime_task *task = runtime_get_running_task();    

    if (!task)
        return 0;

    event_register(index, task, handler);

    return 1;

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

    event_unregister(index, task);

    return 1;

}

unsigned int syscall_execute(char *path, unsigned int argc, char **argv)
{

    struct runtime_task *oldtask = runtime_get_running_task();

    if (!oldtask)
        return 0;

    struct runtime_task *task = runtime_get_slot();

    if (!task)
        return 0;

    task->parentid = oldtask->id;

    if (!task->load(task, path, argc, argv))
        return 0;

    runtime_activate(task);

    event_handler(EVENT_SYSCALL_EXECUTE);

    return 1;

}

unsigned int syscall_exit()
{

    struct runtime_task *oldtask = runtime_get_running_task();

    if (!oldtask)
        return 0;

    oldtask->unload(oldtask);

    struct runtime_task *task = runtime_get_task(oldtask->parentid);

    if (!task)
        return 0;

    runtime_activate(task);

    event_handler(EVENT_SYSCALL_EXIT);

    return 1;

}

unsigned int syscall_info(unsigned int fd, struct file_info *info)
{

    struct runtime_task *task = runtime_get_running_task();

    if (!task)
        return 0;

    struct vfs_node *node = task->get_descriptor(task, fd)->node;

    if (!node)
        return 0;

    info->id = node->id;
    info->length = 0;

    return 1;

}

unsigned int syscall_load(char *path)
{

    struct vfs_node *node = vfs_find(path);

    if (!node)
        return 0;

    void *entry = runtime_relocate(node->physical);

    if (!entry)
        return 0;

    void (*minit)() = entry;
    minit();

    event_handler(EVENT_SYSCALL_LOAD);

    return 1;

}

unsigned int syscall_open(char *path)
{

    struct runtime_task *task = runtime_get_running_task();    

    if (!task)
        return 0;

    struct vfs_node *node = vfs_find(path);

    if (!node)
        return 0;

    struct vfs_descriptor *descriptor = task->add_descriptor(task, node);

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

