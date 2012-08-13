#include <event.h>
#include <mmu.h>
#include <modules.h>
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
    unsigned int address;

    if (!descriptor || !descriptor->id || !descriptor->mount->filesystem->read)
        return 0;

    slot = runtime_get_task_slot();

    if (!slot)
        return 0;

    ntask = runtime_get_task(slot);

    task->clone(task, ntask, slot);

    entry = binary_get_entry(descriptor->mount->filesystem, descriptor->id);

    if (!entry)
        return 0;

    address = binary_get_vaddress(descriptor->mount->filesystem, descriptor->id);

    if (!address)
        return 0;

    runtime_memory_init(&ntask->memory, address, RUNTIME_TASK_ADDRESS_SIZE);

    mmu_map_user_memory(ntask->id, RUNTIME_TASK_ADDRESS_BASE + ntask->id * RUNTIME_TASK_ADDRESS_SIZE, address, RUNTIME_TASK_ADDRESS_SIZE);
    mmu_load_user_memory(ntask->id);

    if (!binary_copy_program(descriptor->mount->filesystem, descriptor->id))
        return 0;

    ntask->used = 1;
    ntask->idle = 0;
    ntask->event = 0;

    runtime_registers_init(&ntask->registers, entry, ntask->memory.address + ntask->memory.size, ntask->memory.address + ntask->memory.size);

    return slot;

}

unsigned int syscall_exit(struct runtime_task *task, void *stack)
{

    task->used = 0;

    return 1;

}

unsigned int syscall_idle(struct runtime_task *task, void *stack)
{

    task->idle = 1;
    task->event = 0;

    return 1;

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
    if (!binary_relocate(descriptor->mount->filesystem, descriptor->id, physical))
        return 0;

    init = (void (*)())(binary_find_symbol(descriptor->mount->filesystem, descriptor->id, "init"));

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

    get_filesystem = (struct modules_filesystem *(*)())(binary_find_symbol(descriptor->mount->filesystem, descriptor->id, "get_filesystem"));

    if (!get_filesystem)
        return 0;

    runtime_mount_init(mount, args->offset, get_filesystem(), args->count, args->path);

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

    mount = task->find_mount(task, args->path);

    if (!mount)
        return 0;

    id = mount->filesystem->walk(mount->filesystem, mount->filesystem->rootid, args->count - mount->count, args->path + mount->count);

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

    destroy = (void (*)())(binary_find_symbol(descriptor->mount->filesystem, descriptor->id, "destroy"));

    if (!destroy)
        return 0;

    destroy();

    return 1;

}

unsigned int syscall_write(struct runtime_task *task, void *stack)
{

    struct syscall_write_args *args = stack;
    struct runtime_descriptor *descriptor = task->get_descriptor(task, args->index);

    if (!descriptor || !descriptor->id || !descriptor->mount->filesystem->write)
        return 0;

    return descriptor->mount->filesystem->write(descriptor->mount->filesystem, descriptor->id, args->offset, args->count, args->buffer);

}

