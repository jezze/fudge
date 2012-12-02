#include <memory.h>
#include <runtime.h>
#include <vfs.h>

unsigned int runtime_set_task_event(struct runtime_task *task, unsigned int index, unsigned int callback)
{

    if (index >= RUNTIME_TASK_EVENT_SLOTS)
        return 0;

    task->events[index].callback = callback;

    return 1;

}

unsigned int runtime_unset_task_event(struct runtime_task *task, unsigned int index)
{

    if (index >= RUNTIME_TASK_EVENT_SLOTS)
        return 0;

    task->events[index].callback = 0;

    return 1;

}

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

static struct runtime_descriptor *runtime_find_child_descriptor(struct runtime_task *task, struct vfs_interface *interface, unsigned int id)
{

    unsigned int i;

    for (i = 1; i < RUNTIME_TASK_MOUNT_SLOTS; i++)
    {

        if (task->mounts[i].child.interface == interface && task->mounts[i].child.id == id)
            continue;

        if (task->mounts[i].parent.interface == interface && task->mounts[i].parent.id == id)
            return &task->mounts[i].child;

    }

    return 0;

}

struct runtime_descriptor *runtime_set_task_descriptor(struct runtime_task *task, unsigned int index, struct vfs_interface *interface, unsigned int id, unsigned int count, char *path)
{

    struct runtime_descriptor *descriptor = runtime_get_task_descriptor(task, index);
    struct runtime_descriptor *child = runtime_find_child_descriptor(task, interface, id);
    unsigned int length;

    if (child)
        return runtime_set_task_descriptor(task, index, child->interface, child->id, count, path);

    if (!count)
    {

        runtime_init_descriptor(descriptor, interface, id);

        return descriptor;

    }

    for (length = 0; length < count && path[length] != '/'; length++);

    if (length < count)
        length++;

    if ((id = interface->walk(interface, id, length, path)))
        return runtime_set_task_descriptor(task, index, interface, id, count - length, path + length);

    return 0;

}

struct runtime_mount *runtime_get_task_mount(struct runtime_task *task, unsigned int index)
{

    if (!index || index >= RUNTIME_TASK_MOUNT_SLOTS)
        return 0;

    return &task->mounts[index];

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

void runtime_init_registers(struct runtime_registers *registers, unsigned int ip, unsigned int sp, unsigned int sb, unsigned int status)
{

    memory_clear(registers, sizeof (struct runtime_registers));

    registers->ip = ip;
    registers->sp = sp;
    registers->sb = sb;
    registers->status = status;

}

void runtime_init_task(struct runtime_task *task)
{

    memory_clear(task, sizeof (struct runtime_task));

    task->status.used = 1;
    task->notify_interrupt = notify_interrupt;

}

