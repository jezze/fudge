#include <fudge/kernel.h>
#include "vfs.h"
#include "binary.h"
#include "task.h"
#include "scheduler.h"
#include "container.h"

static struct vfs_descriptor *get_descriptor(struct task *task, unsigned int index)
{

    if (!index || index >= TASK_DESCRIPTORS)
        return 0;

    return &task->descriptors[index];

}

static struct vfs_mount *get_mount(struct container *container, unsigned int index)
{

    if (!index || index >= CONTAINER_MOUNTS)
        return 0;

    return &container->mounts[index];

}

static unsigned int open(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int index; unsigned int pindex; unsigned int count; const char *path;} *args = stack;
    struct vfs_descriptor *descriptor = get_descriptor(task, args->index);
    struct vfs_descriptor *pdescriptor = get_descriptor(task, args->pindex);
    struct vfs_descriptor temp;
    unsigned int offset;
    unsigned int count;

    if (!descriptor || !pdescriptor)
        return 0;

    if (!pdescriptor->id || !pdescriptor->channel)
        return 0;

    memory_copy(&temp, pdescriptor, sizeof (struct vfs_descriptor));

    for (offset = 0; (count = vfs_findnext(args->count - offset, args->path + offset)); offset += count)
    {

        unsigned int i;

        if (vfs_isparent(count, args->path + offset))
        {

            for (i = 1; i < CONTAINER_MOUNTS; i++)
            {

                struct vfs_mount *mount = &self->mounts[i];

                if (temp.channel == mount->child.channel && temp.id == mount->child.id)
                {

                    temp.channel = mount->parent.channel;
                    temp.id = mount->parent.id;

                    break;

                }

            }

            temp.id = temp.channel->protocol->parent(temp.channel->backend, temp.id);

            if (!temp.id)
                return 0;

        }

        else
        {

            temp.id = temp.channel->protocol->walk(temp.channel->backend, temp.id, count, args->path + offset);

            if (!temp.id)
                return 0;

            for (i = 1; i < CONTAINER_MOUNTS; i++)
            {

                struct vfs_mount *mount = &self->mounts[i];

                if (temp.channel == mount->parent.channel && temp.id == mount->parent.id)
                {

                    temp.channel = mount->child.channel;
                    temp.id = mount->child.id;

                    break;

                }

            }

        }

    }

    memory_copy(descriptor, &temp, sizeof (struct vfs_descriptor));

    return descriptor->id = descriptor->channel->protocol->open(descriptor->channel->backend, descriptor->id);

}

static unsigned int close(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int index;} *args = stack;
    struct vfs_descriptor *descriptor = get_descriptor(task, args->index);

    if (!descriptor)
        return 0;

    if (!descriptor->id || !descriptor->channel)
        return 0;

    return descriptor->id = descriptor->channel->protocol->close(descriptor->channel->backend, descriptor->id);

}

static unsigned int read(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int index; unsigned int offset; unsigned int count; void *buffer;} *args = stack;
    struct vfs_descriptor *descriptor = get_descriptor(task, args->index);

    if (!descriptor)
        return 0;

    if (!descriptor->id || !descriptor->channel)
        return 0;

    return descriptor->channel->protocol->read(descriptor->channel->backend, descriptor->id, args->offset, args->count, args->buffer);

}

static unsigned int write(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int index; unsigned int offset; unsigned int count; void *buffer;} *args = stack;
    struct vfs_descriptor *descriptor = get_descriptor(task, args->index);

    if (!descriptor)
        return 0;

    if (!descriptor->id || !descriptor->channel)
        return 0;

    return descriptor->channel->protocol->write(descriptor->channel->backend, descriptor->id, args->offset, args->count, args->buffer);

}

static unsigned int mount(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int index; unsigned int pindex; unsigned int cindex;} *args = stack;
    struct vfs_channel *channel = &self->channels[0x02];
    struct vfs_mount *mount = get_mount(self, args->index);
    struct vfs_descriptor *pdescriptor = get_descriptor(task, args->pindex);
    struct vfs_descriptor *cdescriptor = get_descriptor(task, args->cindex);
    struct binary_protocol *protocol;
    struct vfs_backend *(*get_backend)();

    if (!mount || !pdescriptor || !cdescriptor)
        return 0;

    if (!pdescriptor->id || !pdescriptor->channel)
        return 0;

    if (!cdescriptor->id || !cdescriptor->channel)
        return 0;

    protocol = binary_find_protocol(cdescriptor->channel, cdescriptor->id);

    if (!protocol)
        return 0;

    get_backend = (struct vfs_backend *(*)())(protocol->find_symbol(cdescriptor->channel, cdescriptor->id, 11, "get_backend"));

    if (!get_backend)
        return 0;

    channel->backend = get_backend();

    if (!channel->backend)
        return 0;

    channel->protocol = vfs_find_protocol(channel->backend);

    if (!channel->protocol)
        return 0;

    mount->parent.channel = pdescriptor->channel;
    mount->parent.id = pdescriptor->id;
    mount->child.channel = channel;
    mount->child.id = channel->protocol->root(channel->backend);

    if (!mount->child.id)
        return 0;

    return 1;

}

static unsigned int bind(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int index; unsigned int pindex; unsigned int cindex;} *args = stack;
    struct vfs_mount *mount = get_mount(self, args->index);
    struct vfs_descriptor *pdescriptor = get_descriptor(task, args->pindex);
    struct vfs_descriptor *cdescriptor = get_descriptor(task, args->cindex);

    if (!mount || !pdescriptor || !cdescriptor)
        return 0;

    if (!pdescriptor->id || !pdescriptor->channel)
        return 0;

    if (!cdescriptor->id || !cdescriptor->channel)
        return 0;

    mount->parent.channel = pdescriptor->channel;
    mount->parent.id = pdescriptor->id;
    mount->child.channel = cdescriptor->channel;
    mount->child.id = cdescriptor->id;

    return 1;

}

static unsigned int execute(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int index;} *args = stack;
    struct vfs_descriptor *descriptor = get_descriptor(task, args->index);
    struct binary_protocol *protocol;

    if (!descriptor)
        return 0;

    if (!descriptor->id || !descriptor->channel)
        return 0;

    protocol = binary_find_protocol(descriptor->channel, descriptor->id);

    if (!protocol)
        return 0;

    task->registers.ip = protocol->copy_program(descriptor->channel, descriptor->id);

    if (!task->registers.ip)
        return 0;

    return task->registers.ip;

}

static unsigned int exit(struct container *self, struct task *task, void *stack)
{

    scheduler_unuse(task);

    return 0;

}

static unsigned int load(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int index;} *args = stack;
    struct vfs_descriptor *descriptor = get_descriptor(task, args->index);
    struct binary_protocol *protocol;
    unsigned long physical;
    void (*init)();

    if (!descriptor)
        return 0;

    if (!descriptor->id || !descriptor->channel)
        return 0;

    if (!descriptor->channel->protocol->get_physical)
        return 0;

    /* Physical should be replaced with known address later on */
    physical = descriptor->channel->protocol->get_physical(descriptor->channel->backend, descriptor->id);

    if (!physical)
        return 0;

    protocol = binary_find_protocol(descriptor->channel, descriptor->id);

    if (!protocol)
        return 0;

    if (!protocol->relocate(descriptor->channel, descriptor->id, physical))
        return 0;

    init = (void (*)())(protocol->find_symbol(descriptor->channel, descriptor->id, 4, "init"));

    if (!init)
        return 0;

    init();

    return 1;

}

static unsigned int unload(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int index;} *args = stack;
    struct vfs_descriptor *descriptor = get_descriptor(task, args->index);
    struct binary_protocol *protocol;
    void (*destroy)();

    if (!descriptor)
        return 0;

    if (!descriptor->id || !descriptor->channel)
        return 0;

    protocol = binary_find_protocol(descriptor->channel, descriptor->id);

    if (!protocol)
        return 0;

    destroy = (void (*)())(protocol->find_symbol(descriptor->channel, descriptor->id, 7, "destroy"));

    if (!destroy)
        return 0;

    destroy();

    return 1;

}

void container_init(struct container *container)
{

    memory_clear(container, sizeof (struct container));

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

