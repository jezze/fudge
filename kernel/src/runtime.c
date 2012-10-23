#include <memory.h>
#include <runtime.h>

struct runtime_task *running;

struct runtime_task *runtime_get_task()
{

    return running;

}

void runtime_set_task(struct runtime_task *task)
{

    running = task;

}

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

static void notify_interrupt(struct runtime_task *task, unsigned int index)
{

}

static void notify_complete(struct runtime_task *task)
{

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

void runtime_init_registers(struct runtime_registers *registers, unsigned int ip, unsigned int sp, unsigned int sb, unsigned int status)
{

    memory_clear(registers, sizeof (struct runtime_registers));

    registers->ip = ip;
    registers->sp = sp;
    registers->sb = sb;
    registers->status = status;

}

void runtime_init_task(struct runtime_task *task, unsigned int ip)
{

    memory_clear(task, sizeof (struct runtime_task));

    runtime_init_registers(&task->registers, ip, RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE, RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE, 0);

    task->notify_interrupt = notify_interrupt;
    task->notify_complete = notify_complete;

}

void runtime_reset_task(struct runtime_task *task, unsigned int ip)
{

    runtime_init_registers(&task->registers, ip, RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE, RUNTIME_TASK_VADDRESS_BASE + RUNTIME_TASK_ADDRESS_SIZE, 0);

}

