#include <fudge.h>
#include "error.h"
#include "resource.h"
#include "vfs.h"
#include "binary.h"
#include "task.h"
#include "scheduler.h"
#include "container.h"
#include "kernel.h"

#define CALLS                           16

static unsigned int (*calls[CALLS])(struct container *container, struct task *task, void *stack);

static struct vfs_descriptor *getdescriptor(struct task *task, unsigned int descriptor)
{

    return (descriptor < TASK_DESCRIPTORS) ? &task->descriptors[descriptor] : 0;

}

static struct vfs_channel *getchannel(struct container *container, unsigned int channel)
{

    return (channel < CONTAINER_CHANNELS) ? &container->channels[channel] : 0;

}

static struct vfs_mount *getmount(struct container *container, unsigned int mount)
{

    return (mount < CONTAINER_MOUNTS) ? &container->mounts[mount] : 0;

}

static unsigned int walk(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int pdescriptor; unsigned int count; const char *path;} *args = stack;
    struct vfs_descriptor *descriptor = getdescriptor(task, args->descriptor);
    struct vfs_descriptor *pdescriptor = getdescriptor(task, args->pdescriptor);
    unsigned int offset;
    unsigned int count;

    if (!descriptor)
        return 0;

    if (!pdescriptor || !pdescriptor->id || !pdescriptor->channel)
        return 0;

    descriptor->channel = pdescriptor->channel;
    descriptor->id = pdescriptor->id;

    for (offset = 0; (count = memory_findbyte(args->path + offset, args->count - offset, '/')); offset += count)
    {

        const char *path = args->path + offset;
        unsigned int i;

        if (count == 3 && memory_match(path, "../", 3))
        {

            for (i = 0; i < CONTAINER_MOUNTS; i++)
            {

                struct vfs_mount *mount = &container->mounts[i];

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

                struct vfs_mount *mount = &container->mounts[i];

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

static unsigned int create(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int pdescriptor; unsigned int count; const char *name;} *args = stack;
    struct vfs_descriptor *pdescriptor = getdescriptor(task, args->pdescriptor);

    if (!pdescriptor || !pdescriptor->id || !pdescriptor->channel)
        return 0;

    return pdescriptor->channel->protocol->create(pdescriptor->channel->backend, pdescriptor->id, args->count, args->name);

}

static unsigned int destroy(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int pdescriptor; unsigned int count; const char *name;} *args = stack;
    struct vfs_descriptor *pdescriptor = getdescriptor(task, args->pdescriptor);

    if (!pdescriptor || !pdescriptor->id || !pdescriptor->channel)
        return 0;

    return pdescriptor->channel->protocol->destroy(pdescriptor->channel->backend, pdescriptor->id, args->count, args->name);

}

static unsigned int open(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct vfs_descriptor *descriptor = getdescriptor(task, args->descriptor);

    if (!descriptor || !descriptor->id || !descriptor->channel)
        return 0;

    return descriptor->id = descriptor->channel->protocol->open(descriptor->channel->backend, descriptor->id);

}

static unsigned int close(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct vfs_descriptor *descriptor = getdescriptor(task, args->descriptor);

    if (!descriptor || !descriptor->id || !descriptor->channel)
        return 0;

    return descriptor->id = descriptor->channel->protocol->close(descriptor->channel->backend, descriptor->id);

}

static unsigned int read(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int offset; unsigned int count; void *buffer;} *args = stack;
    struct vfs_descriptor *descriptor = getdescriptor(task, args->descriptor);

    if (!descriptor || !descriptor->id || !descriptor->channel)
        return 0;

    return descriptor->channel->protocol->read(descriptor->channel->backend, descriptor->id, args->offset, args->count, args->buffer);

}

static unsigned int write(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int offset; unsigned int count; void *buffer;} *args = stack;
    struct vfs_descriptor *descriptor = getdescriptor(task, args->descriptor);

    if (!descriptor || !descriptor->id || !descriptor->channel)
        return 0;

    return descriptor->channel->protocol->write(descriptor->channel->backend, descriptor->id, args->offset, args->count, args->buffer);

}

static unsigned int auth(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int channel; unsigned int backend;} *args = stack;
    struct vfs_channel *channel = getchannel(container, args->channel);
    struct vfs_backend *backend = vfs_findbackend(args->backend);
    struct vfs_protocol *protocol;

    if (!channel || !backend)
        return 0;

    protocol = vfs_findprotocol(backend);

    if (!protocol)
        return 0;

    channel->backend = backend;
    channel->protocol = protocol;

    return 1;

}

static unsigned int mount(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int mount; unsigned int channel; unsigned int descriptor;} *args = stack;
    struct vfs_mount *mount = getmount(container, args->mount);
    struct vfs_channel *channel = getchannel(container, args->channel);
    struct vfs_descriptor *pdescriptor = getdescriptor(task, args->descriptor);

    if (!mount)
        return 0;

    if (!channel || !channel->backend || !channel->protocol)
        return 0;

    if (!pdescriptor || !pdescriptor->id || !pdescriptor->channel)
        return 0;

    mount->parent.channel = pdescriptor->channel;
    mount->parent.id = pdescriptor->id;
    mount->child.channel = channel;
    mount->child.id = channel->protocol->root(channel->backend);

    return 1;

}

static unsigned int bind(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int mount; unsigned int pdescriptor; unsigned int cdescriptor;} *args = stack;
    struct vfs_mount *mount = getmount(container, args->mount);
    struct vfs_descriptor *pdescriptor = getdescriptor(task, args->pdescriptor);
    struct vfs_descriptor *cdescriptor = getdescriptor(task, args->cdescriptor);

    if (!mount)
        return 0;

    if (!pdescriptor || !pdescriptor->id || !pdescriptor->channel)
        return 0;

    if (!cdescriptor || !cdescriptor->id || !cdescriptor->channel)
        return 0;

    mount->parent.channel = pdescriptor->channel;
    mount->parent.id = pdescriptor->id;
    mount->child.channel = cdescriptor->channel;
    mount->child.id = cdescriptor->id;

    return 1;

}

static unsigned int execute(struct container *container, struct task *task, void *stack)
{

    struct vfs_descriptor *descriptor = getdescriptor(task, 2);
    struct binary_protocol *protocol;

    if (!descriptor || !descriptor->id || !descriptor->channel)
        return 0;

    protocol = binary_findprotocol(descriptor->channel, descriptor->id);

    if (!protocol)
        return 0;

    return protocol->copyprogram(descriptor->channel, descriptor->id);

}

static unsigned int load(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct vfs_descriptor *descriptor = getdescriptor(task, args->descriptor);
    struct binary_protocol *protocol;
    unsigned long physical;
    void (*module_init)();
    void (*module_register)();

    if (!descriptor || !descriptor->id || !descriptor->channel)
        return 0;

    if (!descriptor->channel->protocol->getphysical)
        return 0;

    /* Physical should be replaced with known address later on */
    physical = descriptor->channel->protocol->getphysical(descriptor->channel->backend, descriptor->id);

    if (!physical)
        return 0;

    protocol = binary_findprotocol(descriptor->channel, descriptor->id);

    if (!protocol)
        return 0;

    if (!protocol->relocate(descriptor->channel, descriptor->id, physical))
        return 0;

    module_init = (void (*)())(protocol->findsymbol(descriptor->channel, descriptor->id, 11, "module_init"));
    module_register = (void (*)())(protocol->findsymbol(descriptor->channel, descriptor->id, 15, "module_register"));

    if (!module_init || !module_register)
        return 0;

    module_init();
    module_register();

    return 1;

}

static unsigned int unload(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct vfs_descriptor *descriptor = getdescriptor(task, args->descriptor);
    struct binary_protocol *protocol;
    void (*module_unregister)();

    if (!descriptor || !descriptor->id || !descriptor->channel)
        return 0;

    protocol = binary_findprotocol(descriptor->channel, descriptor->id);

    if (!protocol)
        return 0;

    module_unregister = (void (*)())(protocol->findsymbol(descriptor->channel, descriptor->id, 17, "module_unregister"));

    if (!module_unregister)
        return 0;

    module_unregister();

    return 1;

}

unsigned int kernel_call(unsigned int index, struct container *container, struct task *task, void *stack)
{

    return calls[index](container, task, stack);

}

void kernel_setupmodules(struct container *container, struct task *task, unsigned int count, struct kernel_module *modules)
{

    struct vfs_channel *channel = &container->channels[0];
    struct vfs_mount *mount = &container->mounts[0];
    struct vfs_descriptor *root = &task->descriptors[0];
    struct vfs_descriptor *work = &task->descriptors[1];
    struct vfs_descriptor *init = &task->descriptors[2];
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        struct binary_protocol *protocol;

        channel->backend = &modules[i].base;
        channel->protocol = vfs_findprotocol(channel->backend);

        if (!channel->protocol)
            continue;

        mount->parent.channel = channel;
        mount->parent.id = channel->protocol->root(channel->backend);
        mount->child.channel = mount->parent.channel;
        mount->child.id = mount->parent.id;

        if (!mount->parent.id)
            continue;

        root->channel = mount->parent.channel;
        root->id = mount->parent.id;
        work->channel = root->channel;
        work->id = root->id;
        init->channel = work->channel;
        init->id = work->channel->protocol->child(work->channel->backend, work->id, 4, "bin/");

        if (!init->id)
            continue;

        init->id = init->channel->protocol->child(init->channel->backend, init->id, 4, "init");

        if (!init->id)
            continue;

        protocol = binary_findprotocol(init->channel, init->id);

        if (!protocol)
            continue;

        task->state.registers.ip = protocol->copyprogram(init->channel, init->id);

    }

    error_assert(task->state.registers.ip != 0, "Failed to locate entry point", __FILE__, __LINE__);

}

void kernel_setup(unsigned int (*spawn)(struct container *container, struct task *task, void *stack), unsigned int (*despawn)(struct container *container, struct task *task, void *stack))
{

    resource_setup();
    scheduler_setup();
    binary_setupelf();
    vfs_setupcpio();
    vfs_setuptar();

    calls[KERNEL_WALK] = walk;
    calls[KERNEL_CREATE] = create;
    calls[KERNEL_DESTROY] = destroy;
    calls[KERNEL_OPEN] = open;
    calls[KERNEL_CLOSE] = close;
    calls[KERNEL_READ] = read;
    calls[KERNEL_WRITE] = write;
    calls[KERNEL_AUTH] = auth;
    calls[KERNEL_MOUNT] = mount;
    calls[KERNEL_BIND] = bind;
    calls[KERNEL_EXECUTE] = execute;
    calls[KERNEL_LOAD] = load;
    calls[KERNEL_UNLOAD] = unload;
    calls[KERNEL_SPAWN] = spawn;
    calls[KERNEL_DESPAWN] = despawn;

}

