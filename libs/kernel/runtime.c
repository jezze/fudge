#include <fudge/kernel.h>
#include "runtime.h"
#include "vfs.h"

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

static struct runtime_descriptor *follow_child(struct runtime_container *container, struct vfs_interface *interface, unsigned int id)
{

    unsigned int i;

    for (i = 1; i < RUNTIME_CONTAINER_MOUNT_SLOTS; i++)
    {

        if (container->mounts[i].parent.interface == interface && container->mounts[i].parent.id == id)
            return &container->mounts[i].child;

    }

    return 0;

}

static struct runtime_descriptor *follow_parent(struct runtime_container *container, struct vfs_interface *interface, unsigned int id)
{

    unsigned int i;

    for (i = 1; i < RUNTIME_CONTAINER_MOUNT_SLOTS; i++)
    {

        if (container->mounts[i].child.interface == interface && container->mounts[i].child.id == id)
            return &container->mounts[i].parent;

    }

    return 0;

}

static unsigned int get_directory_length(unsigned int count, const char *path)
{

    unsigned int length;

    for (length = 0; length <= count && !(length > 0 && path[length - 1] == '/'); length++);

    if (length > count)
        length = count;

    return length;

}

static unsigned int is_parent(unsigned int count, const char *path)
{

    return count >= 3 && memory_match(path, "../", 3);

}

unsigned int runtime_update_descriptor(struct runtime_task *task, struct runtime_descriptor *descriptor, struct vfs_interface *interface, unsigned int id, unsigned int count, const char *path)
{

    for (;;)
    {

        struct runtime_descriptor *temp = is_parent(count, path) ? follow_parent(task->container, interface, id) : follow_child(task->container, interface, id);
        unsigned int length;

        if (temp)
        {

            id = temp->id;
            interface = temp->interface;

            continue;

        }

        if (!count)
            break;

        length = get_directory_length(count, path);
        id = interface->walk(interface, id, length, path);

        if (!id)
            return 0;

        count -= length;
        path += length;

    };

    descriptor->id = id;
    descriptor->interface = interface;

    return descriptor->id;

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

