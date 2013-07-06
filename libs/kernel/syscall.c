#include <fudge/kernel.h>
#include "vfs.h"
#include "binary.h"
#include "runtime.h"
#include "syscall.h"

#define SYSCALL_ROUTINE_SLOTS           16

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
    unsigned int n;

    if (!descriptor || !pdescriptor)
        return 0;

    memory_copy(descriptor, pdescriptor, sizeof (struct runtime_descriptor));

    while ((n = vfs_findnext(args->count, args->path)))
    {

        descriptor->id = descriptor->session.protocol->walk(descriptor->session.backend, descriptor->id, n, args->path);

        if (!descriptor->id)
            return 0;

        pdescriptor = runtime_get_child(task->container, descriptor);

        if (pdescriptor)
            memory_copy(descriptor, pdescriptor, sizeof (struct runtime_descriptor));

        args->count -= n;
        args->path += n;

    };

    return 1;
    /*
    return descriptor->session.protocol->open(descriptor->session.backend, descriptor->id);
    */

}

static unsigned int close(struct runtime_task *task, void *stack)
{

    struct {void *caller; unsigned int index;} *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_descriptor(task, args->index);

    if (!descriptor)
        return 0;

    return descriptor->session.protocol->close(descriptor->session.backend, descriptor->id);

}

static unsigned int read(struct runtime_task *task, void *stack)
{

    struct {void *caller; unsigned int index; unsigned int offset; unsigned int count; void *buffer;} *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_descriptor(task, args->index);

    if (!descriptor)
        return 0;

    return descriptor->session.protocol->read(descriptor->session.backend, descriptor->id, args->offset, args->count, args->buffer);

}

static unsigned int write(struct runtime_task *task, void *stack)
{

    struct {void *caller; unsigned int index; unsigned int offset; unsigned int count; void *buffer;} *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_descriptor(task, args->index);

    if (!descriptor)
        return 0;

    return descriptor->session.protocol->write(descriptor->session.backend, descriptor->id, args->offset, args->count, args->buffer);

}

static unsigned int mount(struct runtime_task *task, void *stack)
{

    struct {void *caller; unsigned int index; unsigned int pindex; unsigned int cindex;} *args = stack;
    struct runtime_mount *mount = runtime_get_mount(task, args->index);
    struct runtime_descriptor *pdescriptor = runtime_get_descriptor(task, args->pindex);
    struct runtime_descriptor *cdescriptor = runtime_get_descriptor(task, args->cindex);
    struct binary_protocol *protocol;
    struct vfs_backend *(*get_backend)();

    if (!mount || !pdescriptor || !cdescriptor)
        return 0;

    protocol = binary_get_protocol(&cdescriptor->session, cdescriptor->id);

    if (!protocol)
        return 0;

    get_backend = (struct vfs_backend *(*)())(protocol->find_symbol(&cdescriptor->session, cdescriptor->id, 11, "get_backend"));

    if (!get_backend)
        return 0;

    mount->child.session.backend = get_backend();

    if (!mount->child.session.backend)
        return 0;

    mount->child.session.protocol = vfs_get_protocol(mount->child.session.backend);

    if (!mount->child.session.protocol)
        return 0;

    mount->child.id = mount->child.session.protocol->rootid;

    memory_copy(&mount->parent, pdescriptor, sizeof (struct runtime_descriptor));

    return 1;

}

static unsigned int bind(struct runtime_task *task, void *stack)
{

    struct {void *caller; unsigned int index; unsigned int pindex; unsigned int cindex;} *args = stack;
    struct runtime_mount *mount = runtime_get_mount(task, args->index);
    struct runtime_descriptor *pdescriptor = runtime_get_descriptor(task, args->pindex);
    struct runtime_descriptor *cdescriptor = runtime_get_descriptor(task, args->cindex);

    if (!mount || !pdescriptor || !cdescriptor)
        return 0;

    memory_copy(&mount->parent, pdescriptor, sizeof (struct runtime_descriptor));
    memory_copy(&mount->child, cdescriptor, sizeof (struct runtime_descriptor));

    return 1;

}

static unsigned int execute(struct runtime_task *task, void *stack)
{

    struct {void *caller; unsigned int index;} *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_descriptor(task, args->index);
    struct binary_protocol *protocol;

    if (!descriptor)
        return 0;

    protocol = binary_get_protocol(&descriptor->session, descriptor->id);

    if (!protocol)
        return 0;

    task->state |= RUNTIME_TASK_STATE_USED;
    task->registers.ip = protocol->copy_program(&descriptor->session, descriptor->id);
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
    struct binary_protocol *protocol;
    unsigned int physical;
    void (*init)();

    if (!descriptor || !descriptor->session.protocol->get_physical)
        return 0;

    /* Physical should be replaced with known address later on */
    physical = descriptor->session.protocol->get_physical(descriptor->session.backend, descriptor->id);

    if (!physical)
        return 0;

    protocol = binary_get_protocol(&descriptor->session, descriptor->id);

    if (!protocol)
        return 0;

    if (!protocol->relocate(&descriptor->session, descriptor->id, physical))
        return 0;

    init = (void (*)())(protocol->find_symbol(&descriptor->session, descriptor->id, 4, "init"));

    if (!init)
        return 0;

    init();

    return 1;

}

static unsigned int unload(struct runtime_task *task, void *stack)
{

    struct {void *caller; unsigned int index;} *args = stack;
    struct runtime_descriptor *descriptor = runtime_get_descriptor(task, args->index);
    struct binary_protocol *protocol;
    void (*destroy)();

    if (!descriptor)
        return 0;

    protocol = binary_get_protocol(&descriptor->session, descriptor->id);

    if (!protocol)
        return 0;

    destroy = (void (*)())(protocol->find_symbol(&descriptor->session, descriptor->id, 7, "destroy"));

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
    syscall_set_routine(SYSCALL_INDEX_BIND, bind);
    syscall_set_routine(SYSCALL_INDEX_EXECUTE, execute);
    syscall_set_routine(SYSCALL_INDEX_EXIT, exit);
    syscall_set_routine(SYSCALL_INDEX_LOAD, load);
    syscall_set_routine(SYSCALL_INDEX_UNLOAD, unload);

}

