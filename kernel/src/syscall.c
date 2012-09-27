#include <memory.h>
#include <vfs.h>
#include <binary.h>
#include <runtime.h>
#include <syscall.h>

static unsigned int (*routines[SYSCALL_TABLE_SLOTS])(struct runtime_task *task, void *stack);

static unsigned int attach(struct runtime_task *task, void *stack)
{

    struct syscall_attach_args *args = stack;

    if (!args->callback)
        return 0;

    return runtime_set_task_event(task, args->index, args->callback);

}

static unsigned int close(struct runtime_task *task, void *stack)
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

static unsigned int detach(struct runtime_task *task, void *stack)
{

    struct syscall_detach_args *args = stack;

    return runtime_unset_task_event(task, args->index);

}

static unsigned int execute(struct runtime_task *task, void *stack)
{

    struct syscall_execute_args *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_task_descriptor(task, args->index);
    unsigned int entry;

    if (!descriptor || !descriptor->interface->read)
        return 0;

    entry = binary_copy_program(descriptor->interface, descriptor->id);

    if (!entry)
        return 0;

    runtime_reset_task(task, entry);

    return task->id;

}

static unsigned int exit(struct runtime_task *task, void *stack)
{

    task->status.used = 0;

    return 1;

}

static unsigned int idle(struct runtime_task *task, void *stack)
{

    task->status.idle = 1;
    task->status.event = 0;

    return 1;

}

static unsigned int load(struct runtime_task *task, void *stack)
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

static unsigned int mount(struct runtime_task *task, void *stack)
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

    runtime_init_mount(mount, get_interface(), args->count, args->path);

    return 1;

}

static unsigned int open(struct runtime_task *task, void *stack)
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

    runtime_init_descriptor(descriptor, id, mount->interface);

    if (descriptor->interface->open)
        descriptor->interface->open(descriptor->interface, descriptor->id);

    return args->index;

}

static unsigned int read(struct runtime_task *task, void *stack)
{

    struct syscall_read_args *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_task_descriptor(task, args->index);

    if (!descriptor || !descriptor->interface->read)
        return 0;

    return descriptor->interface->read(descriptor->interface, descriptor->id, args->offset, args->count, args->buffer);

}

static unsigned int undefined(struct runtime_task *task, void *stack)
{

    return 0;

}

static unsigned int unload(struct runtime_task *task, void *stack)
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

static unsigned int write(struct runtime_task *task, void *stack)
{

    struct syscall_write_args *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_task_descriptor(task, args->index);

    if (!descriptor || !descriptor->interface->write)
        return 0;

    return descriptor->interface->write(descriptor->interface, descriptor->id, args->offset, args->count, args->buffer);

}

void syscall_set_routine(unsigned int index, unsigned int (*routine)(struct runtime_task *task, void *stack))
{

    if (index >= SYSCALL_TABLE_SLOTS)
        return;

    routines[index] = routine;

}

void syscall_unset_routine(unsigned int index)
{

    if (index >= SYSCALL_TABLE_SLOTS)
        return;

    routines[index] = 0;

}

unsigned int syscall_raise(unsigned int index, void *stack)
{

    struct runtime_task *task = runtime_get_task();

    if (index >= SYSCALL_TABLE_SLOTS)
        return 0;

    return routines[index](task, stack);

}

void syscall_setup()
{

    unsigned int i;

    for (i = 0; i < SYSCALL_TABLE_SLOTS; i++)
        syscall_set_routine(i, undefined);

    syscall_set_routine(SYSCALL_INDEX_OPEN, open);
    syscall_set_routine(SYSCALL_INDEX_CLOSE, close);
    syscall_set_routine(SYSCALL_INDEX_READ, read);
    syscall_set_routine(SYSCALL_INDEX_WRITE, write);
    syscall_set_routine(SYSCALL_INDEX_MOUNT, mount);
    syscall_set_routine(SYSCALL_INDEX_EXECUTE, execute);
    syscall_set_routine(SYSCALL_INDEX_EXIT, exit);
    syscall_set_routine(SYSCALL_INDEX_IDLE, idle);
    syscall_set_routine(SYSCALL_INDEX_LOAD, load);
    syscall_set_routine(SYSCALL_INDEX_UNLOAD, unload);
    syscall_set_routine(SYSCALL_INDEX_ATTACH, attach);
    syscall_set_routine(SYSCALL_INDEX_DETACH, detach);

}

