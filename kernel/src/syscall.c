#include <event.h>
#include <mmu.h>
#include <modules.h>
#include <runtime.h>
#include <binary.h>
#include <syscall.h>

unsigned int syscall_attach(struct runtime_task *task, void *stack)
{

    struct syscall_attach_args *args = stack;

    if (!args->callback)
        return 0;

    return event_register_routine(args->index, task, args->callback);

}

unsigned int syscall_close(struct runtime_task *task, void *stack)
{

    struct syscall_close_args *args = stack;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);

    if (!descriptor || !descriptor->id)
        return 0;

    if (descriptor->mount->filesystem->close)
        descriptor->mount->filesystem->close(descriptor->mount->filesystem, descriptor->id);

    descriptor->id = 0;

    return 1;

}

unsigned int syscall_detach(struct runtime_task *task, void *stack)
{

    struct syscall_detach_args *args = stack;

    return event_unregister_routine(args->index, task);

}

unsigned int syscall_execute(struct runtime_task *task, void *stack)
{

    struct syscall_execute_args *args = stack;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);
    struct runtime_task *ntask;
    unsigned int slot;
    unsigned int entry;
    unsigned int vaddress;
    unsigned int count;

    if (!descriptor || !descriptor->id || !descriptor->mount->filesystem->read)
        return 0;

    slot = runtime_get_task_slot();

    if (!slot)
        return 0;

    entry = binary_get_entry(descriptor);

    if (!entry)
        return 0;

    vaddress = binary_get_vaddress(descriptor);

    if (!vaddress)
        return 0;

    ntask = runtime_get_task(slot);

    runtime_task_clone(ntask, task, slot);
    ntask->memory.vaddress = vaddress;

    runtime_registers_init(&ntask->registers, entry, ntask->memory.vaddress + ntask->memory.size, ntask->memory.vaddress + ntask->memory.size);

    mmu_map_user_memory(ntask->id, ntask->memory.paddress, ntask->memory.vaddress, ntask->memory.size);
    mmu_load_user_memory(ntask->id);

    count = descriptor->mount->filesystem->read(descriptor->mount->filesystem, descriptor->id, 0, ntask->memory.size, (void *)ntask->memory.vaddress);

    if (!count)
        return 0;

    task->wait = 1;
    ntask->parent = task;
    ntask->used = 1;
    ntask->wait = 0;

    return slot;

}

unsigned int syscall_exit(struct runtime_task *task, void *stack)
{

    task->used = 0;
    task->wait = 1;

    if (task->parent && task->parent->id)
    {

        task->parent->wait = 0;

        return task->parent->id;

    }

    return 0;

}

unsigned int syscall_load(struct runtime_task *task, void *stack)
{

    struct syscall_load_args *args = stack;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);
    void (*init)();
    unsigned int physical;

    if (!descriptor || !descriptor->id || !descriptor->mount->filesystem->get_physical)
        return 0;

    physical = descriptor->mount->filesystem->get_physical(descriptor->mount->filesystem, descriptor->id);

    if (!physical)
        return 0;

    /* Physical should be replaced with known address later on */
    binary_relocate(descriptor, physical);

    init = (void (*)())(binary_find_symbol(descriptor, "init"));

    if (!init)
        return 0;

    init();

    return 1;

}

unsigned int syscall_mount(struct runtime_task *task, void *stack)
{

    struct syscall_mount_args *args = stack;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);
    struct runtime_mount *mount;
    struct modules_filesystem *(*get_filesystem)();

    if (!descriptor || !descriptor->id)
        return 0;

    mount = task->get_mount(task, args->offset);

    if (!mount)
        return 0;

    get_filesystem = (struct modules_filesystem *(*)())(binary_find_symbol(descriptor, "get_filesystem"));

    if (!get_filesystem)
        return 0;

    runtime_mount_init(mount, args->offset, get_filesystem(), args->count, args->buffer);

    return 1;

}

unsigned int syscall_open(struct runtime_task *task, void *stack)
{

    struct syscall_open_args *args = stack;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);
    struct runtime_mount *mount;
    unsigned int id;

    if (!descriptor)
        return 0;

    if (!args->count)
        return 0;

    mount = task->find_mount(task, args->buffer);

    if (!mount)
        return 0;

    id = mount->filesystem->walk(mount->filesystem, descriptor->id, args->count - mount->count, (char *)args->buffer + mount->count);

    if (!id)
        return 0;

    runtime_descriptor_init(descriptor, id, mount);

    if (descriptor->mount->filesystem->open)
        descriptor->mount->filesystem->open(descriptor->mount->filesystem, descriptor->id);

    return args->index;

}

unsigned int syscall_read(struct runtime_task *task, void *stack)
{

    struct syscall_read_args *args = stack;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);

    if (!descriptor || !descriptor->id || !descriptor->mount->filesystem->read)
        return 0;

    return descriptor->mount->filesystem->read(descriptor->mount->filesystem, descriptor->id, args->offset, args->count, args->buffer);

}

unsigned int syscall_unload(struct runtime_task *task, void *stack)
{

    struct syscall_unload_args *args = stack;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);
    void (*destroy)();

    if (!descriptor || !descriptor->id)
        return 0;

    destroy = (void (*)())(binary_find_symbol(descriptor, "destroy"));

    if (!destroy)
        return 0;

    destroy();

    return 1;

}

unsigned int syscall_wait(struct runtime_task *task, void *stack)
{

    task->event = 0;
    task->wait = 1;

    if (task->parent && task->parent->id)
    {

        task->parent->wait = 0;

        return task->parent->id;

    }

    return 0;

}

unsigned int syscall_write(struct runtime_task *task, void *stack)
{

    struct syscall_write_args *args = stack;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);

    if (!descriptor || !descriptor->id || !descriptor->mount->filesystem->write)
        return 0;

    return descriptor->mount->filesystem->write(descriptor->mount->filesystem, descriptor->id, args->offset, args->count, args->buffer);

}

