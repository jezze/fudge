#include <memory.h>
#include <runtime.h>

static struct runtime_task tasks[RUNTIME_TASK_SLOTS];

struct runtime_task *runtime_get_task(unsigned int index)
{

    if (!index || index >= RUNTIME_TASK_SLOTS)
        return 0;

    return &tasks[index];

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

unsigned int runtime_get_task_slot(unsigned int parent)
{

    unsigned int i;

    for (i = parent; i < RUNTIME_TASK_SLOTS; i++)
    {

        if (!tasks[i].status.used)
            return i;

    }

    return 0;

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

void runtime_descriptor_init(struct runtime_descriptor *descriptor, unsigned int id, struct runtime_mount *mount)
{

    memory_clear(descriptor, sizeof (struct runtime_descriptor));

    descriptor->id = id;
    descriptor->mount = mount;

}

void runtime_mount_init(struct runtime_mount *mount, struct vfs_interface *interface, unsigned int count, char *path)
{

    memory_clear(mount, sizeof (struct runtime_mount));

    mount->interface = interface;
    mount->count = count;
    memory_copy(mount->path, path, mount->count);

}

void runtime_task_init(struct runtime_task *task, unsigned int id, unsigned int ip)
{

    memory_clear(task, sizeof (struct runtime_task));

    task->id = id;
    task->registers.ip = ip;
    task->registers.sp = RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE;
    task->registers.sb = RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE;

}

void runtime_task_clone(struct runtime_task *from, struct runtime_task *to, unsigned int id, unsigned int ip)
{

    memory_copy(to, from, sizeof (struct runtime_task));

    to->id = id;
    to->registers.ip = ip;
    to->registers.sp = RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE;
    to->registers.sb = RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE;

}

struct runtime_task *runtime_schedule()
{

    unsigned int i;

    for (i = RUNTIME_TASK_SLOTS - 1; i > 0; i--)
    {

        if (tasks[i].status.used && !tasks[i].status.idle)
            return &tasks[i];

    }

    return 0;

}

struct runtime_task *runtime_setup()
{

    memory_clear(tasks, sizeof (struct runtime_task) * RUNTIME_TASK_SLOTS);

    return tasks;

}

