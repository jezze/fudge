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

    event_register(index, task->id, handler);

    return 1;

}

unsigned int syscall_close(unsigned int fd)
{

    struct runtime_task *task = runtime_get_running_task();    

    if (!task)
        return 0;

    task->remove_descriptor(task, fd);

    return 1;

}

unsigned int syscall_detach(unsigned int index)
{

    struct runtime_task *task = runtime_get_running_task();    

    if (!task)
        return 0;

    event_unregister(index, task->id);

    return 1;

}

unsigned int syscall_execute(char *path, unsigned int argc, char **argv, void *ip, void *sp, void *sb)
{

    struct runtime_task *oldtask = runtime_get_running_task();

    if (!oldtask)
        return 0;

    struct runtime_task *task = runtime_get_free_task();

    if (!task)
        return 0;

    task->parentid = oldtask->id;

    if (!task->load(task, path, argc, argv))
        return 0;

    runtime_activate(task);

    event_handler_syscall(0x03);

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

    runtime_relocate(node->physical);

    void (*minit)() = node->physical + 0x40;
    minit();

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

    return node->read(node, count, buffer);

}

unsigned int syscall_reboot()
{

    kernel_reboot();

    return 1;

}

unsigned int syscall_unload()
{

    return 1;

}

unsigned int syscall_wait(void *ip, void *sp, void *sb)
{

    struct runtime_task *oldtask = runtime_get_running_task();

    if (!oldtask)
        return 0;

    struct runtime_task *task = runtime_get_task(oldtask->parentid);

    if (!task)
        return 0;

    runtime_activate(task);

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

    return node->write(node, count, buffer);

}

