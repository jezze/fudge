#include <memory.h>
#include <string.h>
#include <vfs.h>
#include <binary.h>
#include <runtime.h>
#include <syscall.h>

static unsigned int (*routines[SYSCALL_ROUTINE_SLOTS])(struct runtime_task *task, void *stack);

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

    runtime_init_registers(&task->registers, entry, RUNTIME_STACK_VADDRESS_BASE, RUNTIME_STACK_VADDRESS_BASE, 0);

    return 1;

}

static unsigned int exit(struct runtime_task *task, void *stack)
{

    task->status.used = 0;

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
    struct runtime_mount *mount = runtime_get_task_mount(task, args->index);
    struct runtime_descriptor *pdescriptor = runtime_get_task_descriptor(task, args->pindex);
    struct runtime_descriptor *cdescriptor = runtime_get_task_descriptor(task, args->cindex);
    struct vfs_interface *(*get_interface)();
    struct vfs_interface *child;

    if (!mount || !pdescriptor || !cdescriptor)
        return 0;

    get_interface = (struct vfs_interface *(*)())(binary_find_symbol(cdescriptor->interface, cdescriptor->id, "get_filesystem"));

    if (!get_interface)
        return 0;

    child = get_interface();

    runtime_init_mount(mount, pdescriptor->interface, pdescriptor->id, child, child->rootid);

    return 1;

}

static unsigned int open(struct runtime_task *task, void *stack)
{

    struct syscall_open_args *args = stack;
    struct runtime_mount *mount = runtime_get_task_mount(task, 1);
    struct runtime_descriptor *descriptor = runtime_get_task_descriptor(task, args->index);
    
    runtime_init_descriptor(descriptor, mount->parent.interface, mount->parent.id);

    if (!runtime_update_task_descriptor(task, descriptor, args->count - 1, args->path + 1))
        return 0;

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

    if (!index || index >= SYSCALL_ROUTINE_SLOTS)
        return;

    routines[index] = routine;

}

void syscall_unset_routine(unsigned int index)
{

    if (!index || index >= SYSCALL_ROUTINE_SLOTS)
        return;

    routines[index] = 0;

}

unsigned int syscall_raise(unsigned int index, struct runtime_task *task, void *stack)
{

    if (!index || index >= SYSCALL_ROUTINE_SLOTS)
        return 0;

    return routines[index](task, stack);

}

void syscall_setup()
{

    unsigned int i;

    for (i = 0; i < SYSCALL_ROUTINE_SLOTS; i++)
        syscall_set_routine(i, undefined);

    syscall_set_routine(SYSCALL_INDEX_OPEN, open);
    syscall_set_routine(SYSCALL_INDEX_CLOSE, close);
    syscall_set_routine(SYSCALL_INDEX_READ, read);
    syscall_set_routine(SYSCALL_INDEX_WRITE, write);
    syscall_set_routine(SYSCALL_INDEX_MOUNT, mount);
    syscall_set_routine(SYSCALL_INDEX_EXECUTE, execute);
    syscall_set_routine(SYSCALL_INDEX_EXIT, exit);
    syscall_set_routine(SYSCALL_INDEX_LOAD, load);
    syscall_set_routine(SYSCALL_INDEX_UNLOAD, unload);

}

