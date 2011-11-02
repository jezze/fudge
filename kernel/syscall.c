#include <lib/file.h>
#include <kernel/vfs.h>
#include <kernel/event.h>
#include <kernel/kernel.h>
#include <kernel/runtime.h>

void syscall_attach(unsigned int index, void (*handler)())
{

    struct runtime_task *task = runtime_get_running_task();    

    event_register(index, task->pid, handler);

}

void syscall_close(unsigned int fd)
{

    struct runtime_task *task = runtime_get_running_task();    

    task->remove_descriptor(task, fd);

}

void syscall_detach(unsigned int index)
{

    struct runtime_task *task = runtime_get_running_task();    

    event_unregister(index, task->pid);

}

unsigned int syscall_info(unsigned int fd, struct file_info *info)
{

    struct runtime_task *task = runtime_get_running_task();

    struct vfs_node *node = task->get_descriptor(task, fd)->node;

    if (!node)
        return 0;

    info->id = node->id;
    info->length = 0;

    return 1;

}

void syscall_load(char *path)
{

    struct vfs_node *node = vfs_find(path);

    runtime_relocate(node->physical);

    void (*minit)() = node->physical + 0x40;
    minit();

}

unsigned int syscall_open(char *path)
{

    struct runtime_task *task = runtime_get_running_task();    

    struct vfs_node *node = vfs_find(path);

    if (!node)
        return 0;

    struct vfs_descriptor *descriptor = task->add_descriptor(task, node);

    if (!descriptor)
        return 0;

    return descriptor->index;

}

unsigned int syscall_read(unsigned int fd, unsigned int count, char *buffer)
{

    struct runtime_task *task = runtime_get_running_task();    

    struct vfs_node *node = task->get_descriptor(task, fd)->node;

    if (!(node && node->read))
        return 0;

    return node->read(node, count, buffer);

}

void syscall_reboot()
{

    kernel_reboot();

}

void syscall_unload()
{

}

unsigned int syscall_write(unsigned int fd, unsigned int count, char *buffer)
{

    struct runtime_task *task = runtime_get_running_task();    

    struct vfs_node *node = task->get_descriptor(task, fd)->node;

    if (!(node && node->write))
        return 0;

    return node->write(node, count, buffer);

}

