#include <fudge/memory.h>
#include <runtime.h>
#include <vfs.h>

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

static struct runtime_descriptor *runtime_find_child_descriptor(struct runtime_task *task, struct runtime_descriptor *descriptor)
{

    unsigned int i;

    for (i = 1; i < RUNTIME_TASK_MOUNT_SLOTS; i++)
    {

        if (task->mounts[i].parent.interface == descriptor->interface && task->mounts[i].parent.id == descriptor->id)
            return &task->mounts[i].child;

    }

    return 0;

}

unsigned int runtime_update_task_descriptor(struct runtime_task *task, struct runtime_descriptor *descriptor, unsigned int count, char *path)
{

    struct runtime_descriptor *child = runtime_find_child_descriptor(task, descriptor);
    unsigned int length;

    if (child)
    {

        descriptor->id = child->id;
        descriptor->interface = child->interface;

        return runtime_update_task_descriptor(task, descriptor, count, path);

    }

    if (!count)
        return 1;

    for (length = 0; length < count && path[length] != '/'; length++);

    if (length < count)
        length++;

    if ((descriptor->id = descriptor->interface->walk(descriptor->interface, descriptor->id, length, path)))
        return runtime_update_task_descriptor(task, descriptor, count - length, path + length);

    return 0;

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

void runtime_init_registers(struct runtime_registers *registers, unsigned int ip, unsigned int sp, unsigned int fp, unsigned int status)
{

    memory_clear(registers, sizeof (struct runtime_registers));

    registers->ip = ip;
    registers->sp = sp;
    registers->fp = fp;
    registers->status = status;

}

void runtime_init_task(struct runtime_task *task)
{

    memory_clear(task, sizeof (struct runtime_task));

    task->status.used = 1;
    task->notify_interrupt = notify_interrupt;

}

