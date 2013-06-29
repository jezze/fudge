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

struct runtime_mount *runtime_get_mount(struct runtime_task *task, unsigned int index)
{

    if (!index || index >= RUNTIME_CONTAINER_MOUNT_SLOTS)
        return 0;

    return &task->container->mounts[index];

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

void runtime_init_task(struct runtime_task *task, struct runtime_container *container)
{

    memory_clear(task, sizeof (struct runtime_task));

    task->state = RUNTIME_TASK_STATE_USED;
    task->container = container;

}

void runtime_init_container(struct runtime_container *container)
{

    memory_clear(container, sizeof (struct runtime_container));

}

