#include <memory.h>
#include <runtime.h>

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

struct runtime_mount *runtime_find_task_mount(struct runtime_task *self, char *path)
{

    unsigned int i;
    unsigned int max = 0;
    struct runtime_mount *current = 0;

    for (i = 1; i < RUNTIME_TASK_MOUNT_SLOTS; i++)
    {

        if (!self->mounts[i].count)
            continue;

        if (self->mounts[i].count < max)
            continue;

        if (memory_match(self->mounts[i].path, path, self->mounts[i].count))
        {

            current = &self->mounts[i];
            max = self->mounts[i].count;

        }

    }

    return current;

}

void runtime_init_descriptor(struct runtime_descriptor *descriptor, unsigned int id, struct vfs_interface *interface)
{

    memory_clear(descriptor, sizeof (struct runtime_descriptor));

    descriptor->id = id;
    descriptor->interface = interface;

}

void runtime_init_mount(struct runtime_mount *mount, struct vfs_interface *interface, unsigned int count, char *path)
{

    memory_clear(mount, sizeof (struct runtime_mount));

    mount->interface = interface;
    mount->count = count;
    memory_copy(mount->path, path, mount->count);

}

void runtime_init_task(struct runtime_task *task, unsigned int id, unsigned int ip)
{

    memory_clear(task, sizeof (struct runtime_task));

    task->id = id;
    task->registers.ip = ip;
    task->registers.sp = RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE;
    task->registers.sb = RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE;

}

void runtime_reset_task(struct runtime_task *task, unsigned int ip)
{

    task->registers.ip = ip;
    task->registers.sp = RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE;
    task->registers.sb = RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE;

}

