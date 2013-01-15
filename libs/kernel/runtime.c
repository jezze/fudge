#include <fudge/kernel.h>
#include "runtime.h"
#include "vfs.h"

static struct runtime_task *notify_interrupt(struct runtime_task *self, unsigned int index)
{

    return self;

}

struct runtime_descriptor *runtime_get_task_descriptor(struct runtime_task *task, unsigned int index)
{

    if (!index || index >= RUNTIME_TASK_DESCRIPTOR_SLOTS)
        return 0;

    return &task->descriptors[index];

}

struct runtime_mount *runtime_get_task_mount(struct runtime_task *task, unsigned int index)
{

    if (!index || index >= RUNTIME_TASK_MOUNT_SLOTS)
        return 0;

    return &task->mounts[index];

}

unsigned int runtime_update_child_descriptor(struct runtime_task *task, struct runtime_descriptor *descriptor)
{

    unsigned int i;

    for (i = 1; i < RUNTIME_TASK_MOUNT_SLOTS; i++)
    {

        if (task->mounts[i].child.interface != descriptor->interface || task->mounts[i].child.id != descriptor->id)
            continue;

        descriptor->id = task->mounts[i].parent.id;
        descriptor->interface = task->mounts[i].parent.interface;

        return i;

    }

    return 0;

}

unsigned int runtime_update_parent_descriptor(struct runtime_task *task, struct runtime_descriptor *descriptor)
{

    unsigned int i;

    for (i = 1; i < RUNTIME_TASK_MOUNT_SLOTS; i++)
    {

        if (task->mounts[i].parent.interface != descriptor->interface || task->mounts[i].parent.id != descriptor->id)
            continue;

        descriptor->id = task->mounts[i].child.id;
        descriptor->interface = task->mounts[i].child.interface;

        return i;

    }

    return 0;

}

unsigned int runtime_update_task_descriptor(struct runtime_task *task, struct runtime_descriptor *descriptor, unsigned int count, const char *path)
{

    while (count)
    {

        char *offset = memory_find(path, "/", count, 1);
        unsigned int length = (offset) ? (unsigned int)(offset - path) + 1 : count;
        unsigned int id = descriptor->interface->walk(descriptor->interface, descriptor->id, length, path);
        unsigned int mount;

        if (id)
        {

            count -= length;
            path += length;
            descriptor->id = id;

            continue;

        }

        if (memory_match(path, "../", 3))
            mount = runtime_update_child_descriptor(task, descriptor);
        else
            mount = runtime_update_parent_descriptor(task, descriptor);

        if (!mount)
            return 0;

    }

    runtime_update_parent_descriptor(task, descriptor);

    return descriptor->id;

}

void runtime_set_registers(struct runtime_task *task, unsigned int ip, unsigned int sp, unsigned int fp, unsigned int status)
{

    task->registers.ip = ip;
    task->registers.sp = sp;
    task->registers.fp = fp;
    task->registers.status = status;

}

void runtime_init_descriptor(struct runtime_descriptor *descriptor, struct vfs_interface *interface, unsigned int id)
{

    memory_clear(descriptor, sizeof (struct runtime_descriptor));

    descriptor->interface = interface;
    descriptor->id = id;

}

void runtime_init_mount(struct runtime_mount *mount, struct vfs_interface *parent, unsigned int parentid, struct vfs_interface *child, unsigned int childid)
{

    memory_clear(mount, sizeof (struct runtime_mount));

    mount->parent.interface = parent;
    mount->parent.id = parentid;
    mount->child.interface = child;
    mount->child.id = childid;

}

void runtime_init_task(struct runtime_task *task)
{

    memory_clear(task, sizeof (struct runtime_task));

    task->status.used = 1;
    task->notify_interrupt = notify_interrupt;

}

