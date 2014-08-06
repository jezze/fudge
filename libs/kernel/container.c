#include <kernel.h>
#include "resource.h"
#include "vfs.h"
#include "binary.h"
#include "task.h"
#include "container.h"

static struct vfs_descriptor *get_descriptor(struct task *task, unsigned int descriptor)
{

    return (descriptor && descriptor < TASK_DESCRIPTORS) ? &task->descriptors[descriptor] : 0;

}

static struct vfs_channel *get_channel(struct container *container, unsigned int channel)
{

    return (channel && channel < CONTAINER_CHANNELS) ? &container->channels[channel] : 0;

}

static struct vfs_mount *get_mount(struct container *container, unsigned int mount)
{

    return (mount && mount < CONTAINER_MOUNTS) ? &container->mounts[mount] : 0;

}

static unsigned int get_walkstep(unsigned int count, const char *path)
{

    unsigned int offset = memory_findbyte(path, count, '/');

    return offset == count ? offset : offset + 1;

}

static unsigned int walk(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int pdescriptor; unsigned int count; const char *path;} *args = stack;
    struct vfs_descriptor *descriptor = get_descriptor(task, args->descriptor);
    struct vfs_descriptor *pdescriptor = get_descriptor(task, args->pdescriptor);
    unsigned int offset;
    unsigned int count;

    if (!descriptor || descriptor->active)
        return 0;

    if (!pdescriptor || !pdescriptor->id || !pdescriptor->channel)
        return 0;

    descriptor->channel = pdescriptor->channel;
    descriptor->id = pdescriptor->id;

    for (offset = 0; (count = get_walkstep(args->count - offset, args->path + offset)); offset += count)
    {

        const char *path = args->path + offset;
        unsigned int i;

        if (count == 3 && memory_match(path, "../", 3))
        {

            for (i = 0; i < CONTAINER_MOUNTS; i++)
            {

                struct vfs_mount *mount = &self->mounts[i];

                if (descriptor->channel == mount->child.channel && descriptor->id == mount->child.id)
                {

                    descriptor->channel = mount->parent.channel;
                    descriptor->id = mount->parent.id;

                    break;

                }

            }

            descriptor->id = descriptor->channel->protocol->parent(descriptor->channel->backend, descriptor->id);

            if (!descriptor->id)
                return 0;

        }

        else
        {

            descriptor->id = descriptor->channel->protocol->child(descriptor->channel->backend, descriptor->id, count, args->path + offset);

            if (!descriptor->id)
                return 0;

            for (i = 0; i < CONTAINER_MOUNTS; i++)
            {

                struct vfs_mount *mount = &self->mounts[i];

                if (descriptor->channel == mount->parent.channel && descriptor->id == mount->parent.id)
                {

                    descriptor->channel = mount->child.channel;
                    descriptor->id = mount->child.id;

                    break;

                }

            }

        }

    }

    return descriptor->id;

}

static unsigned int create(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int pdescriptor; unsigned int count; const char *name;} *args = stack;
    struct vfs_descriptor *pdescriptor = get_descriptor(task, args->pdescriptor);

    if (!pdescriptor || !pdescriptor->id || !pdescriptor->channel)
        return 0;

    return pdescriptor->channel->protocol->create(pdescriptor->channel->backend, pdescriptor->id, args->count, args->name);

}

static unsigned int destroy(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int pdescriptor; unsigned int count; const char *name;} *args = stack;
    struct vfs_descriptor *pdescriptor = get_descriptor(task, args->pdescriptor);

    if (!pdescriptor || !pdescriptor->id || !pdescriptor->channel)
        return 0;

    return pdescriptor->channel->protocol->destroy(pdescriptor->channel->backend, pdescriptor->id, args->count, args->name);

}

static unsigned int open(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct vfs_descriptor *descriptor = get_descriptor(task, args->descriptor);

    if (!descriptor || !descriptor->id || !descriptor->channel)
        return 0;

    descriptor->active++;

    return descriptor->id = descriptor->channel->protocol->open(descriptor->channel->backend, descriptor->id);

}

static unsigned int close(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct vfs_descriptor *descriptor = get_descriptor(task, args->descriptor);

    if (!descriptor || !descriptor->id || !descriptor->channel)
        return 0;

    descriptor->active--;

    return descriptor->id = descriptor->channel->protocol->close(descriptor->channel->backend, descriptor->id);

}

static unsigned int read(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int offset; unsigned int count; void *buffer;} *args = stack;
    struct vfs_descriptor *descriptor = get_descriptor(task, args->descriptor);

    if (!descriptor || !descriptor->id || !descriptor->channel || !descriptor->active)
        return 0;

    return descriptor->channel->protocol->read(descriptor->channel->backend, descriptor->id, args->offset, args->count, args->buffer);

}

static unsigned int write(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int offset; unsigned int count; void *buffer;} *args = stack;
    struct vfs_descriptor *descriptor = get_descriptor(task, args->descriptor);

    if (!descriptor || !descriptor->id || !descriptor->channel || !descriptor->active)
        return 0;

    return descriptor->channel->protocol->write(descriptor->channel->backend, descriptor->id, args->offset, args->count, args->buffer);

}

static unsigned int auth(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int channel; unsigned int backend;} *args = stack;
    struct vfs_channel *channel = get_channel(self, args->channel);
    struct vfs_backend *backend = vfs_find_backend(args->backend);
    struct vfs_protocol *protocol;

    if (!channel || !backend)
        return 0;

    protocol = vfs_find_protocol(backend);

    if (!protocol)
        return 0;

    channel->backend = backend;
    channel->protocol = protocol;

    return 1;

}

static unsigned int mount(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int mount; unsigned int channel; unsigned int descriptor;} *args = stack;
    struct vfs_mount *mount = get_mount(self, args->mount);
    struct vfs_channel *channel = get_channel(self, args->channel);
    struct vfs_descriptor *pdescriptor = get_descriptor(task, args->descriptor);

    if (!mount)
        return 0;

    if (!channel || !channel->backend || !channel->protocol)
        return 0;

    if (!pdescriptor || !pdescriptor->id || !pdescriptor->channel || !pdescriptor->active)
        return 0;

    mount->parent.channel = pdescriptor->channel;
    mount->parent.id = pdescriptor->id;
    mount->child.channel = channel;
    mount->child.id = channel->protocol->root(channel->backend);

    return 1;

}

static unsigned int bind(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int mount; unsigned int pdescriptor; unsigned int cdescriptor;} *args = stack;
    struct vfs_mount *mount = get_mount(self, args->mount);
    struct vfs_descriptor *pdescriptor = get_descriptor(task, args->pdescriptor);
    struct vfs_descriptor *cdescriptor = get_descriptor(task, args->cdescriptor);

    if (!mount)
        return 0;

    if (!pdescriptor || !pdescriptor->id || !pdescriptor->channel || !pdescriptor->active)
        return 0;

    if (!cdescriptor || !cdescriptor->id || !cdescriptor->channel || !cdescriptor->active)
        return 0;

    mount->parent.channel = pdescriptor->channel;
    mount->parent.id = pdescriptor->id;
    mount->child.channel = cdescriptor->channel;
    mount->child.id = cdescriptor->id;

    return 1;

}

static unsigned int execute(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct vfs_descriptor *descriptor = get_descriptor(task, args->descriptor);
    struct binary_protocol *protocol;

    if (!descriptor || !descriptor->id || !descriptor->channel)
        return 0;

    protocol = binary_find_protocol(descriptor->channel, descriptor->id);

    if (!protocol)
        return 0;

    return protocol->copy_program(descriptor->channel, descriptor->id);

}

static unsigned int load(struct container *self, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct vfs_descriptor *descriptor = get_descriptor(task, args->descriptor);
    struct binary_protocol *protocol;
    unsigned long physical;
    void (*init)();

    if (!descriptor || !descriptor->id || !descriptor->channel || !descriptor->active)
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

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct vfs_descriptor *descriptor = get_descriptor(task, args->descriptor);
    struct binary_protocol *protocol;
    void (*destroy)();

    if (!descriptor || !descriptor->id || !descriptor->channel || !descriptor->active)
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

void container_init(struct container *container, unsigned int (*spawn)(struct container *self, struct task *task, void *stack), unsigned int (*exit)(struct container *self, struct task *task, void *stack))
{

    unsigned int i;

    memory_clear(container, sizeof (struct container));
    resource_init(&container->resource, CONTAINER_RESOURCE, container);

    for (i = 0; i < CONTAINER_CHANNELS; i++)
        vfs_init_channel(&container->channels[i]);

    for (i = 0; i < CONTAINER_MOUNTS; i++)
        vfs_init_mount(&container->mounts[i]);

    container->calls[CONTAINER_CALL_WALK] = walk;
    container->calls[CONTAINER_CALL_CREATE] = create;
    container->calls[CONTAINER_CALL_DESTROY] = destroy;
    container->calls[CONTAINER_CALL_OPEN] = open;
    container->calls[CONTAINER_CALL_CLOSE] = close;
    container->calls[CONTAINER_CALL_READ] = read;
    container->calls[CONTAINER_CALL_WRITE] = write;
    container->calls[CONTAINER_CALL_AUTH] = auth;
    container->calls[CONTAINER_CALL_MOUNT] = mount;
    container->calls[CONTAINER_CALL_BIND] = bind;
    container->calls[CONTAINER_CALL_EXECUTE] = execute;
    container->calls[CONTAINER_CALL_LOAD] = load;
    container->calls[CONTAINER_CALL_UNLOAD] = unload;
    container->calls[CONTAINER_CALL_SPAWN] = spawn;
    container->calls[CONTAINER_CALL_EXIT] = exit;

}

