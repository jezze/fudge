#include <fudge/kernel.h>
#include "vfs.h"
#include "binary.h"
#include "runtime.h"
#include "syscall.h"

static unsigned int (*routines[SYSCALL_ROUTINE_SLOTS])(struct runtime_task *task, void *stack);

static unsigned int undefined(struct runtime_task *task, void *stack)
{

    return 0;

}

static unsigned int open(struct runtime_task *task, void *stack)
{

    struct {void *caller; unsigned int index; unsigned int pindex; unsigned int count; const char *path;} *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_descriptor(task, args->index);
    struct runtime_descriptor *pdescriptor = runtime_get_descriptor(task, args->pindex);
    unsigned int length;

    if (!descriptor || !pdescriptor)
        return 0;

    descriptor->interface = pdescriptor->interface;
    descriptor->id = pdescriptor->id;

    for (;;)
    {

        pdescriptor = (args->count >= 3 && memory_match(args->path, "../", 3)) ? runtime_get_parent(task->container, descriptor->interface, descriptor->id) : runtime_get_child(task->container, descriptor->interface, descriptor->id);

        if (pdescriptor)
        {

            descriptor->id = pdescriptor->id;
            descriptor->interface = pdescriptor->interface;

            continue;

        }

        if (!args->count)
            break;

        for (length = 0; length <= args->count && !(length > 0 && args->path[length - 1] == '/'); length++);

        if (length > args->count)
            length = args->count;

        descriptor->id = descriptor->interface->walk(descriptor->interface, descriptor->id, length, args->path);

        if (!descriptor->id)
            return 0;

        args->count -= length;
        args->path += length;

    };

    return descriptor->interface->open(descriptor->interface, descriptor->id);

}

static unsigned int close(struct runtime_task *task, void *stack)
{

    struct {void *caller; unsigned int index;} *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_descriptor(task, args->index);

    if (!descriptor)
        return 0;

    return descriptor->interface->close(descriptor->interface, descriptor->id);

}

static unsigned int read(struct runtime_task *task, void *stack)
{

    struct {void *caller; unsigned int index; unsigned int offset; unsigned int count; void *buffer;} *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_descriptor(task, args->index);

    if (!descriptor)
        return 0;

    return descriptor->interface->read(descriptor->interface, descriptor->id, args->offset, args->count, args->buffer);

}

static unsigned int write(struct runtime_task *task, void *stack)
{

    struct {void *caller; unsigned int index; unsigned int offset; unsigned int count; void *buffer;} *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_descriptor(task, args->index);

    if (!descriptor)
        return 0;

    return descriptor->interface->write(descriptor->interface, descriptor->id, args->offset, args->count, args->buffer);

}

static unsigned int mount(struct runtime_task *task, void *stack)
{

    struct {void *caller; unsigned int index; unsigned int pindex; unsigned int cindex;} *args = stack;
    struct runtime_mount *mount = runtime_get_mount(task, args->index);
    struct runtime_descriptor *pdescriptor = runtime_get_descriptor(task, args->pindex);
    struct runtime_descriptor *cdescriptor = runtime_get_descriptor(task, args->cindex);
    struct binary_format *format;
    struct vfs_interface *(*get_interface)();
    struct vfs_interface *child;

    if (!mount || !pdescriptor || !cdescriptor)
        return 0;

    format = binary_get_format(cdescriptor->interface, cdescriptor->id);

    if (!format)
        return 0;

    get_interface = (struct vfs_interface *(*)())(format->find_symbol(cdescriptor->interface, cdescriptor->id, 14, "get_filesystem"));

    if (!get_interface)
        return 0;

    child = get_interface();

    if (!child)
        return 0;

    mount->parent.interface = pdescriptor->interface;
    mount->parent.id = pdescriptor->id;
    mount->child.interface = child;
    mount->child.id = child->rootid;

    return 1;

}

static unsigned int execute(struct runtime_task *task, void *stack)
{

    struct {void *caller; unsigned int index;} *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_descriptor(task, args->index);
    struct binary_format *format;

    if (!descriptor)
        return 0;

    format = binary_get_format(descriptor->interface, descriptor->id);

    if (!format)
        return 0;

    task->state |= RUNTIME_TASK_STATE_USED;
    task->registers.ip = format->copy_program(descriptor->interface, descriptor->id);
    task->registers.sp = RUNTIME_STACKADDRESS_VIRTUAL;
    task->registers.fp = RUNTIME_STACKADDRESS_VIRTUAL;
    task->registers.status = 0;

    return task->state;

}

static unsigned int exit(struct runtime_task *task, void *stack)
{

    task->state &= ~RUNTIME_TASK_STATE_USED;
    task->registers.ip = 0;
    task->registers.sp = 0;
    task->registers.fp = 0;
    task->registers.status = 0;

    return task->state;

}

static unsigned int load(struct runtime_task *task, void *stack)
{

    struct {void *caller; unsigned int index;} *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_descriptor(task, args->index);
    struct binary_format *format;
    unsigned int physical;
    void (*init)();

    if (!descriptor || !descriptor->interface->get_physical)
        return 0;

    /* Physical should be replaced with known address later on */
    physical = descriptor->interface->get_physical(descriptor->interface, descriptor->id);

    if (!physical)
        return 0;

    format = binary_get_format(descriptor->interface, descriptor->id);

    if (!format)
        return 0;

    if (!format->relocate(descriptor->interface, descriptor->id, physical))
        return 0;

    init = (void (*)())(format->find_symbol(descriptor->interface, descriptor->id, 4, "init"));

    if (!init)
        return 0;

    init();

    return 1;

}

static unsigned int unload(struct runtime_task *task, void *stack)
{

    struct {void *caller; unsigned int index;} *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_descriptor(task, args->index);
    struct binary_format *format;
    void (*destroy)();

    if (!descriptor)
        return 0;

    format = binary_get_format(descriptor->interface, descriptor->id);

    if (!format)
        return 0;

    destroy = (void (*)())(format->find_symbol(descriptor->interface, descriptor->id, 7, "destroy"));

    if (!destroy)
        return 0;

    destroy();

    return 1;

}

void syscall_set_routine(enum syscall_index index, unsigned int (*routine)(struct runtime_task *task, void *stack))
{

    routines[index] = routine;

}

void syscall_unset_routine(enum syscall_index index)
{

    routines[index] = 0;

}

unsigned int syscall_raise(unsigned int index, struct runtime_task *task)
{

    if (!index || index >= SYSCALL_ROUTINE_SLOTS)
        return 0;

    return routines[index](task, (void *)task->registers.sp);

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

