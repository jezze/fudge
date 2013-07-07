#include <fudge/kernel.h>
#include "vfs.h"
#include "runtime.h"
#include "syscall.h"

struct runtime_descriptor *runtime_get_descriptor(struct runtime_task *task, unsigned int index)
{

    if (!index || index >= RUNTIME_TASK_DESCRIPTOR_SLOTS)
        return 0;

    return &task->descriptors[index];

}

struct runtime_mount *runtime_get_mount(struct runtime_container *container, unsigned int index)
{

    if (!index || index >= RUNTIME_CONTAINER_MOUNT_SLOTS)
        return 0;

    return &container->mounts[index];

}

struct runtime_descriptor *runtime_get_child(struct runtime_container *container, struct runtime_descriptor *descriptor)
{

    unsigned int i;

    for (i = 1; i < RUNTIME_CONTAINER_MOUNT_SLOTS; i++)
    {

        if (memory_match(&container->mounts[i].parent, descriptor, sizeof (struct runtime_descriptor)))
            return &container->mounts[i].child;

    }

    return 0;

}

struct runtime_descriptor *runtime_get_parent(struct runtime_container *container, struct runtime_descriptor *descriptor)
{

    unsigned int i;

    for (i = 1; i < RUNTIME_CONTAINER_MOUNT_SLOTS; i++)
    {

        if (memory_match(&container->mounts[i].child, descriptor, sizeof (struct runtime_descriptor)))
            return &container->mounts[i].parent;

    }

    return 0;

}

void runtime_set_syscall(struct runtime_container *container, unsigned int index, unsigned int (*routine)(struct runtime_container *container, struct runtime_task *task, void *stack))
{

    if (!index || index >= RUNTIME_CONTAINER_SYSCALL_SLOTS)
        return;

    container->syscalls[index] = routine;

}

void runtime_init_task(struct runtime_task *task, unsigned int ip, unsigned int sp, unsigned int fp)
{

    memory_clear(task, sizeof (struct runtime_task));

    task->state = RUNTIME_TASK_STATE_USED;
    task->registers.ip = ip;
    task->registers.sp = sp;
    task->registers.fp = fp;

}

void runtime_init_container(struct runtime_container *container, struct runtime_task *task)
{

    memory_clear(container, sizeof (struct runtime_container));

    container->running = task;

}

