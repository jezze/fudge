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

void runtime_set_task_registers(struct runtime_task *task, unsigned int ip, unsigned int sp, unsigned int fp, unsigned int status)
{

    task->registers.ip = ip;
    task->registers.sp = sp;
    task->registers.fp = fp;
    task->registers.status = status;

}

void runtime_set_task_descriptor(struct runtime_task *task, unsigned int index, struct vfs_interface *interface, unsigned int id)
{

    if (!index || index >= RUNTIME_TASK_DESCRIPTOR_SLOTS)
        return;

    task->descriptors[index].interface = interface;
    task->descriptors[index].id = id;

}

void runtime_set_task_mount(struct runtime_task *task, unsigned int index, struct vfs_interface *parent, unsigned int parentid, struct vfs_interface *child, unsigned int childid)
{

    if (!index || index >= RUNTIME_TASK_MOUNT_SLOTS)
        return;

    task->mounts[index].parent.interface = parent;
    task->mounts[index].parent.id = parentid;
    task->mounts[index].child.interface = child;
    task->mounts[index].child.id = childid;

}

static unsigned int follow_child(struct runtime_task *task, struct runtime_descriptor *descriptor)
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

static unsigned int follow_parent(struct runtime_task *task, struct runtime_descriptor *descriptor)
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

        mount = (memory_match(path, "../", 3)) ? follow_parent(task, descriptor) : follow_child(task, descriptor);

        if (!mount)
            return 0;

    }

    follow_child(task, descriptor);

    return descriptor->id;

}

void runtime_init_task(struct runtime_task *task)
{

    memory_clear(task, sizeof (struct runtime_task));

    task->status.used = 1;
    task->notify_interrupt = notify_interrupt;

}

