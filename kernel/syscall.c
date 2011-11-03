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

unsigned int syscall_execute(char *path, unsigned int argc, char **argv, void *eip, void *esp, void *ebp)
{

    struct runtime_task *oldtask = runtime_get_running_task();
    oldtask->save(oldtask, eip, esp, ebp);

    struct runtime_task *task = runtime_get_free_task();
    task->parentpid = oldtask->pid;

    if (!task->load(task, path, argc, argv))
        return 0;

    runtime_activate(task);

    return 1;

}

void syscall_exit()
{

    struct runtime_task *oldtask = runtime_get_running_task();
    oldtask->unload(oldtask);

    struct runtime_task *task = runtime_get_task(oldtask->parentpid);

    runtime_activate(task);

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

unsigned int syscall_wait(void *eip, void *esp, void *ebp)
{

    struct runtime_task *oldtask = runtime_get_running_task();
    oldtask->save(oldtask, eip, esp, ebp);

    struct runtime_task *task = runtime_get_task(oldtask->parentpid);

    runtime_activate(task);

    return 1;

}

unsigned int syscall_write(unsigned int fd, unsigned int count, char *buffer)
{

    struct runtime_task *task = runtime_get_running_task();    

    struct vfs_node *node = task->get_descriptor(task, fd)->node;

    if (!(node && node->write))
        return 0;

    return node->write(node, count, buffer);

}

