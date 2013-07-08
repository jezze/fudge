#include <fudge/kernel.h>
#include "vfs.h"
#include "binary.h"
#include "task.h"
#include "container.h"

static struct task_descriptor *get_descriptor(struct task *task, unsigned int index)
{

    if (!index || index >= TASK_DESCRIPTORS)
        return 0;

    return &task->descriptors[index];

}

static struct container_mount *get_mount(struct container *container, unsigned int index)
{

    if (!index || index >= CONTAINER_MOUNTS)
        return 0;

    return &container->mounts[index];

}

static struct task_descriptor *get_child(struct container *container, struct task_descriptor *descriptor)
{

    unsigned int i;

    for (i = 1; i < CONTAINER_MOUNTS; i++)
    {

        if (memory_match(&container->mounts[i].parent, descriptor, sizeof (struct task_descriptor)))
            return &container->mounts[i].child;

    }

    return 0;

}

static struct task_descriptor *get_parent(struct container *container, struct task_descriptor *descriptor)
{

    unsigned int i;

    for (i = 1; i < CONTAINER_MOUNTS; i++)
    {

        if (memory_match(&container->mounts[i].child, descriptor, sizeof (struct task_descriptor)))
            return &container->mounts[i].parent;

    }

    return 0;

}

static unsigned int open(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int index; unsigned int pindex; unsigned int count; const char *path;} *args = stack;
    struct task_descriptor *descriptor = get_descriptor(task, args->index);
    struct task_descriptor *pdescriptor = get_descriptor(task, args->pindex);
    struct task_descriptor temp;
    unsigned int n;

    if (!descriptor || !pdescriptor)
        return 0;

    if (!pdescriptor->session.backend || !pdescriptor->session.protocol)
        return 0;

    memory_copy(&temp, pdescriptor, sizeof (struct task_descriptor));

    while ((n = vfs_findnext(args->count, args->path)))
    {

        if (vfs_isparent(n, args->path))
        {

            if ((pdescriptor = get_parent(self, &temp)))
                memory_copy(&temp, pdescriptor, sizeof (struct task_descriptor));

            temp.id = temp.session.protocol->parent(temp.session.backend, temp.id);

            if (!temp.id)
                return 0;

        }

        else
        {

            temp.id = temp.session.protocol->walk(temp.session.backend, temp.id, n, args->path);

            if (!temp.id)
                return 0;

            if ((pdescriptor = get_child(self, &temp)))
                memory_copy(&temp, pdescriptor, sizeof (struct task_descriptor));

        }

        args->count -= n;
        args->path += n;

    };

    memory_copy(descriptor, &temp, sizeof (struct task_descriptor));

    return descriptor->session.protocol->open(descriptor->session.backend, descriptor->id);

}

static unsigned int close(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int index;} *args = stack;
    struct task_descriptor *descriptor = get_descriptor(task, args->index);

    if (!descriptor)
        return 0;

    if (!descriptor->session.backend || !descriptor->session.protocol)
        return 0;

    return descriptor->session.protocol->close(descriptor->session.backend, descriptor->id);

}

static unsigned int read(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int index; unsigned int offset; unsigned int count; void *buffer;} *args = stack;
    struct task_descriptor *descriptor = get_descriptor(task, args->index);

    if (!descriptor)
        return 0;

    if (!descriptor->session.backend || !descriptor->session.protocol)
        return 0;

    return descriptor->session.protocol->read(descriptor->session.backend, descriptor->id, args->offset, args->count, args->buffer);

}

static unsigned int write(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int index; unsigned int offset; unsigned int count; void *buffer;} *args = stack;
    struct task_descriptor *descriptor = get_descriptor(task, args->index);

    if (!descriptor)
        return 0;

    if (!descriptor->session.backend || !descriptor->session.protocol)
        return 0;

    return descriptor->session.protocol->write(descriptor->session.backend, descriptor->id, args->offset, args->count, args->buffer);

}

static unsigned int mount(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int index; unsigned int pindex; unsigned int cindex;} *args = stack;
    struct container_mount *mount = get_mount(self, args->index);
    struct task_descriptor *pdescriptor = get_descriptor(task, args->pindex);
    struct task_descriptor *cdescriptor = get_descriptor(task, args->cindex);
    struct binary_protocol *protocol;
    struct vfs_backend *(*get_backend)();

    if (!mount || !pdescriptor || !cdescriptor)
        return 0;

    if (!pdescriptor->session.backend || !pdescriptor->session.protocol)
        return 0;

    if (!cdescriptor->session.backend || !cdescriptor->session.protocol)
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

    memory_copy(&mount->parent, pdescriptor, sizeof (struct task_descriptor));

    return 1;

}

static unsigned int bind(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int index; unsigned int pindex; unsigned int cindex;} *args = stack;
    struct container_mount *mount = get_mount(self, args->index);
    struct task_descriptor *pdescriptor = get_descriptor(task, args->pindex);
    struct task_descriptor *cdescriptor = get_descriptor(task, args->cindex);

    if (!mount || !pdescriptor || !cdescriptor)
        return 0;

    if (!pdescriptor->session.backend || !pdescriptor->session.protocol)
        return 0;

    if (!cdescriptor->session.backend || !cdescriptor->session.protocol)
        return 0;

    memory_copy(&mount->parent, pdescriptor, sizeof (struct task_descriptor));
    memory_copy(&mount->child, cdescriptor, sizeof (struct task_descriptor));

    return 1;

}

static unsigned int execute(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int index;} *args = stack;
    struct task_descriptor *descriptor = get_descriptor(task, args->index);
    struct binary_protocol *protocol;

    if (!descriptor)
        return 0;

    if (!descriptor->session.backend || !descriptor->session.protocol)
        return 0;

    protocol = binary_get_protocol(&descriptor->session, descriptor->id);

    if (!protocol)
        return 0;

    task->state |= TASK_STATE_USED;
    task->registers.ip = protocol->copy_program(&descriptor->session, descriptor->id);
    task->registers.sp = STACKADDRESS_VIRTUAL;
    task->registers.fp = STACKADDRESS_VIRTUAL;
    task->registers.status = 0;

    return task->state;

}

static unsigned int exit(struct container *self, struct task *task, void *stack)
{

    task->state &= ~TASK_STATE_USED;
    task->registers.ip = 0;
    task->registers.sp = 0;
    task->registers.fp = 0;
    task->registers.status = 0;

    return task->state;

}

static unsigned int load(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int index;} *args = stack;
    struct task_descriptor *descriptor = get_descriptor(task, args->index);
    struct binary_protocol *protocol;
    unsigned int physical;
    void (*init)();

    if (!descriptor)
        return 0;

    if (!descriptor->session.backend || !descriptor->session.protocol)
        return 0;

    if (!descriptor->session.protocol->get_physical)
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

static unsigned int unload(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int index;} *args = stack;
    struct task_descriptor *descriptor = get_descriptor(task, args->index);
    struct binary_protocol *protocol;
    void (*destroy)();

    if (!descriptor)
        return 0;

    if (!descriptor->session.backend || !descriptor->session.protocol)
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

void container_init(struct container *container, struct task *task)
{

    memory_clear(container, sizeof (struct container));

    container->running = task;
    container->calls[CONTAINER_CALL_OPEN] = open;
    container->calls[CONTAINER_CALL_CLOSE] = close;
    container->calls[CONTAINER_CALL_READ] = read;
    container->calls[CONTAINER_CALL_WRITE] = write;
    container->calls[CONTAINER_CALL_MOUNT] = mount;
    container->calls[CONTAINER_CALL_BIND] = bind;
    container->calls[CONTAINER_CALL_EXECUTE] = execute;
    container->calls[CONTAINER_CALL_EXIT] = exit;
    container->calls[CONTAINER_CALL_LOAD] = load;
    container->calls[CONTAINER_CALL_UNLOAD] = unload;

}

