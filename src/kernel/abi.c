#include <fudge.h>
#include "resource.h"
#include "task.h"
#include "vfs.h"
#include "binary.h"
#include "container.h"

#define CALLS                           18

static unsigned int (*calls[CALLS])(struct container *container, struct task *task, void *stack);

static struct vfs_channel *getchannel(struct container *container, unsigned int channel)
{

    return &container->channels[channel];

}

static struct container_mount *getmount(struct container *container, unsigned int mount)
{

    return &container->mounts[mount];

}

static struct container_descriptor *getdescriptor(struct container *container, struct task *task, unsigned int descriptor)
{

    return &container->descriptors[task->id * TASK_DESCRIPTORS + descriptor];

}

static unsigned int walk(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int pdescriptor; unsigned int count; char *path;} *args = stack;
    struct container_descriptor *descriptor = getdescriptor(container, task, args->descriptor);
    struct container_descriptor *pdescriptor = getdescriptor(container, task, args->pdescriptor);
    unsigned int offset;
    unsigned int count;

    if (!pdescriptor->id)
        return 0;

    descriptor->channel = pdescriptor->channel;
    descriptor->id = pdescriptor->id;

    for (offset = 0; (count = memory_findbyte(args->path + offset, args->count - offset, '/')); offset += count)
    {

        if (count == 3 && memory_match(args->path + offset, "../", 3))
        {

            struct vfs_channel *channel;
            unsigned int i;

            for (i = 0; i < CONTAINER_MOUNTS; i++)
            {

                struct container_mount *mount = &container->mounts[i];

                if (descriptor->channel == mount->child.channel && descriptor->id == mount->child.id)
                {

                    descriptor->channel = mount->parent.channel;
                    descriptor->id = mount->parent.id;

                    break;

                }

            }

            channel = getchannel(container, descriptor->channel);
            descriptor->id = channel->protocol->parent(channel->backend, descriptor->id);

            if (!descriptor->id)
                return 0;

        }

        else
        {

            struct vfs_channel *channel;
            unsigned int i;

            channel = getchannel(container, descriptor->channel);
            descriptor->id = channel->protocol->child(channel->backend, descriptor->id, count, args->path + offset);

            if (!descriptor->id)
                return 0;

            for (i = 0; i < CONTAINER_MOUNTS; i++)
            {

                struct container_mount *mount = &container->mounts[i];

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

    struct {void *caller; unsigned int descriptor; unsigned int count; char *name;} *args = stack;
    struct container_descriptor *descriptor = getdescriptor(container, task, args->descriptor);
    struct vfs_channel *channel = getchannel(container, descriptor->channel);

    if (!descriptor->id || !args->count)
        return 0;

    return channel->protocol->create(channel->backend, descriptor->id, args->count, args->name);

}

static unsigned int destroy(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int count; char *name;} *args = stack;
    struct container_descriptor *descriptor = getdescriptor(container, task, args->descriptor);
    struct vfs_channel *channel = getchannel(container, descriptor->channel);

    if (!descriptor->id || !args->count)
        return 0;

    return channel->protocol->destroy(channel->backend, descriptor->id, args->count, args->name);

}

static unsigned int open(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct container_descriptor *descriptor = getdescriptor(container, task, args->descriptor);
    struct vfs_channel *channel = getchannel(container, descriptor->channel);

    if (!descriptor->id)
        return 0;

    return descriptor->id = channel->protocol->open(channel->backend, task, args->descriptor, descriptor->id);

}

static unsigned int close(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct container_descriptor *descriptor = getdescriptor(container, task, args->descriptor);
    struct vfs_channel *channel = getchannel(container, descriptor->channel);

    if (!descriptor->id)
        return 0;

    return descriptor->id = channel->protocol->close(channel->backend, task, args->descriptor, descriptor->id);

}

static unsigned int read(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count;} *args = stack;
    struct container_descriptor *descriptor = getdescriptor(container, task, args->descriptor);
    struct vfs_channel *channel = getchannel(container, descriptor->channel);
    unsigned int count;

    if (!descriptor->id || !args->count)
        return 0;

    count = channel->protocol->read(channel->backend, task, args->descriptor, descriptor->id, descriptor->offset, args->count, args->buffer);
    descriptor->offset += count;

    return count;

}

static unsigned int seekread(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count; unsigned int offset;} *args = stack;
    struct container_descriptor *descriptor = getdescriptor(container, task, args->descriptor);
    struct vfs_channel *channel = getchannel(container, descriptor->channel);

    if (!descriptor->id || !args->count)
        return 0;

    return channel->protocol->read(channel->backend, task, args->descriptor, descriptor->id, args->offset, args->count, args->buffer);

}

static unsigned int write(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count;} *args = stack;
    struct container_descriptor *descriptor = getdescriptor(container, task, args->descriptor);
    struct vfs_channel *channel = getchannel(container, descriptor->channel);
    unsigned int count;

    if (!descriptor->id || !args->count)
        return 0;

    count = channel->protocol->write(channel->backend, task, args->descriptor, descriptor->id, descriptor->offset, args->count, args->buffer);
    descriptor->offset += count;

    return count;

}

static unsigned int seekwrite(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count; unsigned int offset;} *args = stack;
    struct container_descriptor *descriptor = getdescriptor(container, task, args->descriptor);
    struct vfs_channel *channel = getchannel(container, descriptor->channel);

    if (!descriptor->id || !args->count)
        return 0;

    return channel->protocol->write(channel->backend, task, args->descriptor, descriptor->id, args->offset, args->count, args->buffer);

}

static unsigned int auth(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int channel; unsigned int backend;} *args = stack;
    struct container_descriptor *descriptor = getdescriptor(container, task, args->descriptor);
    struct vfs_channel *channel = getchannel(container, args->channel);

    if (!descriptor)
        return 0;

    channel->backend = vfs_findbackend(args->backend);

    if (!channel->backend)
        return 0;

    channel->protocol = vfs_findprotocol(channel->backend);

    if (!channel->protocol)
        return 0;

    descriptor->channel = args->channel;
    descriptor->id = channel->protocol->root(channel->backend);

    if (!descriptor->id)
        return 0;

    return args->channel;

}

static unsigned int mount(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int mount; unsigned int pdescriptor; unsigned int cdescriptor;} *args = stack;
    struct container_mount *mount = getmount(container, args->mount);
    struct container_descriptor *pdescriptor = getdescriptor(container, task, args->pdescriptor);
    struct container_descriptor *cdescriptor = getdescriptor(container, task, args->cdescriptor);

    if (!cdescriptor->id || !pdescriptor->id)
        return 0;

    mount->parent.channel = pdescriptor->channel;
    mount->parent.id = pdescriptor->id;
    mount->child.channel = cdescriptor->channel;
    mount->child.id = cdescriptor->id;

    return 1;

}

static unsigned int load(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct container_descriptor *descriptor = getdescriptor(container, task, args->descriptor);
    struct vfs_channel *channel = getchannel(container, descriptor->channel);
    struct binary_format *format;
    unsigned long physical;
    void (*module_init)(void);
    void (*module_register)(void);

    if (!descriptor->id || !channel->protocol->getphysical)
        return 0;

    /* Physical should be replaced with known address later on */
    physical = channel->protocol->getphysical(channel->backend, descriptor->id);

    if (!physical)
        return 0;

    format = binary_findformat(channel, task, args->descriptor, descriptor->id);

    if (!format || !format->relocate(channel, task, args->descriptor, descriptor->id, physical))
        return 0;

    module_init = (void (*)(void))(format->findsymbol(channel, task, args->descriptor, descriptor->id, 11, "module_init"));

    if (module_init)
        module_init();

    module_register = (void (*)(void))(format->findsymbol(channel, task, args->descriptor, descriptor->id, 15, "module_register"));

    if (module_register)
        module_register();

    return 1;

}

static unsigned int unload(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct container_descriptor *descriptor = getdescriptor(container, task, args->descriptor);
    struct vfs_channel *channel = getchannel(container, descriptor->channel);
    struct binary_format *format;
    void (*module_unregister)(void);

    if (!descriptor->id)
        return 0;

    format = binary_findformat(channel, task, args->descriptor, descriptor->id);

    if (!format)
        return 0;

    module_unregister = (void (*)(void))(format->findsymbol(channel, task, args->descriptor, descriptor->id, 17, "module_unregister"));

    if (module_unregister)
        module_unregister();

    return 1;

}

static unsigned int seek(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int offset;} *args = stack;
    struct container_descriptor *descriptor = getdescriptor(container, task, args->descriptor);

    if (!descriptor->id)
        return 0;

    return descriptor->offset = args->offset;

}

static unsigned int scan(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int index;} *args = stack;
    struct container_descriptor *descriptor = getdescriptor(container, task, args->descriptor);
    struct vfs_channel *channel = getchannel(container, descriptor->channel);

    if (!descriptor->id)
        return 0;

    return channel->protocol->scan(channel->backend, descriptor->id, args->index);

}

unsigned int abi_call(unsigned int index, struct container *container, struct task *task, void *stack)
{

    return calls[index](container, task, stack);

}

void abi_setup(unsigned int (*spawn)(struct container *container, struct task *task, void *stack), unsigned int (*despawn)(struct container *container, struct task *task, void *stack))
{

    calls[0x01] = walk;
    calls[0x02] = create;
    calls[0x03] = destroy;
    calls[0x04] = open;
    calls[0x05] = close;
    calls[0x06] = read;
    calls[0x07] = write;
    calls[0x08] = auth;
    calls[0x09] = mount;
    calls[0x0A] = load;
    calls[0x0B] = unload;
    calls[0x0C] = spawn;
    calls[0x0D] = despawn;
    calls[0x0E] = seek;
    calls[0x0F] = scan;
    calls[0x10] = seekread;
    calls[0x11] = seekwrite;

}

