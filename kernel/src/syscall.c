#include <memory.h>
#include <event.h>
#include <mmu.h>
#include <vfs.h>
#include <binary.h>
#include <runtime.h>
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
    struct runtime_descriptor *descriptor = runtime_get_task_descriptor(task, args->index);

    if (!descriptor)
        return 0;

    if (descriptor->interface->close)
        descriptor->interface->close(descriptor->interface, descriptor->id);

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
    struct runtime_descriptor *descriptor = runtime_get_task_descriptor(task, args->index);
    unsigned int entry;

    if (!descriptor || !descriptor->interface->read)
        return 0;

    entry = binary_get_entry(descriptor->interface, descriptor->id);

    if (!entry)
        return 0;

    if (!binary_copy_program(descriptor->interface, descriptor->id))
        return 0;

    runtime_task_reset(task, entry);

    return task->id;

}

unsigned int syscall_exit(struct runtime_task *task, void *stack)
{

    task->status.used = 0;

    return 1;

}

unsigned int syscall_idle(struct runtime_task *task, void *stack)
{

    task->status.idle = 1;
    task->status.event = 0;

    return 1;

}

unsigned int syscall_load(struct runtime_task *task, void *stack)
{

    struct syscall_load_args *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_task_descriptor(task, args->index);
    void (*init)();
    unsigned int physical;

    if (!descriptor || !descriptor->interface->get_physical)
        return 0;

    physical = descriptor->interface->get_physical(descriptor->interface, descriptor->id);

    if (!physical)
        return 0;

    /* Physical should be replaced with known address later on */
    if (!binary_relocate(descriptor->interface, descriptor->id, physical))
        return 0;

    init = (void (*)())(binary_find_symbol(descriptor->interface, descriptor->id, "init"));

    if (!init)
        return 0;

    init();

    return 1;

}

unsigned int syscall_mount(struct runtime_task *task, void *stack)
{

    struct syscall_mount_args *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_task_descriptor(task, args->index);
    struct runtime_mount *mount = runtime_get_task_mount(task, args->offset);
    struct vfs_interface *(*get_interface)();

    if (!args->count || !args->path || !descriptor || !mount)
        return 0;

    get_interface = (struct vfs_interface *(*)())(binary_find_symbol(descriptor->interface, descriptor->id, "get_filesystem"));

    if (!get_interface)
        return 0;

    runtime_mount_init(mount, get_interface(), args->count, args->path);

    return 1;

}

unsigned int syscall_open(struct runtime_task *task, void *stack)
{

    struct syscall_open_args *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_task_descriptor(task, args->index);
    struct runtime_mount *mount = runtime_find_task_mount(task, args->path);
    unsigned int id;

    if (!args->count || !args->path || !descriptor || !mount)
        return 0;

    id = mount->interface->walk(mount->interface, mount->interface->rootid, args->count - mount->count, args->path + mount->count);

    if (!id)
        return 0;

    runtime_descriptor_init(descriptor, id, mount->interface);

    if (descriptor->interface->open)
        descriptor->interface->open(descriptor->interface, descriptor->id);

    return args->index;

}

unsigned int syscall_read(struct runtime_task *task, void *stack)
{

    struct syscall_read_args *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_task_descriptor(task, args->index);

    if (!descriptor || !descriptor->interface->read)
        return 0;

    return descriptor->interface->read(descriptor->interface, descriptor->id, args->offset, args->count, args->buffer);

}

unsigned int syscall_spawn(struct runtime_task *task, void *stack)
{

    struct syscall_spawn_args *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_task_descriptor(task, args->index);
    struct runtime_task *ntask;
    unsigned int slot;
    unsigned int entry;

    if (!descriptor || !descriptor->interface->read)
        return 0;

    slot = runtime_get_task_slot(task->id);

    if (!slot)
        return 0;

    entry = binary_get_entry(descriptor->interface, descriptor->id);

    if (!entry)
        return 0;

    ntask = runtime_get_task(slot);

    runtime_task_clone(ntask, task, slot, entry);

    mmu_map_user_memory(ntask->id, RUNTIME_TASK_PADDRESS_BASE + ntask->id * RUNTIME_TASK_ADDRESS_SIZE, RUNTIME_TASK_VADDRESS_BASE, RUNTIME_TASK_ADDRESS_SIZE);
    mmu_load_user_memory(ntask->id);

    if (!binary_copy_program(descriptor->interface, descriptor->id))
        return 0;

    ntask->status.used = 1;

    return slot;

}

unsigned int syscall_unload(struct runtime_task *task, void *stack)
{

    struct syscall_unload_args *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_task_descriptor(task, args->index);
    void (*destroy)();

    if (!descriptor)
        return 0;

    destroy = (void (*)())(binary_find_symbol(descriptor->interface, descriptor->id, "destroy"));

    if (!destroy)
        return 0;

    destroy();

    return 1;

}

unsigned int syscall_write(struct runtime_task *task, void *stack)
{

    struct syscall_write_args *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_task_descriptor(task, args->index);

    if (!descriptor || !descriptor->interface->write)
        return 0;

    return descriptor->interface->write(descriptor->interface, descriptor->id, args->offset, args->count, args->buffer);

}

