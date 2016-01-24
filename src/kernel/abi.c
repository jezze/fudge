#include <fudge.h>
#include "resource.h"
#include "task.h"
#include "service.h"
#include "binary.h"
#include "container.h"

#define CALLS                           18

static unsigned int (*calls[CALLS])(struct container *container, struct task *task, void *stack);

static struct service_channel *getchannel(struct container *container, unsigned int channel)
{

    return &container->channels[channel];

}

static struct container_mount *getmount(struct container *container, unsigned int mount)
{

    return &container->mounts[mount];

}

static struct container_session *getsession(struct container *container, struct task *task, unsigned int descriptor)
{

    return &container->sessions[task->id * TASK_DESCRIPTORS + descriptor];

}

static unsigned int walk(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int pdescriptor; unsigned int count; char *path;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);
    struct container_session *psession = getsession(container, task, args->pdescriptor);
    unsigned int offset;
    unsigned int count;

    if (!psession->id)
        return 0;

    session->channel = psession->channel;
    session->id = psession->id;

    for (offset = 0; (count = memory_findbyte(args->path + offset, args->count - offset, '/')); offset += count)
    {

        if (count == 3 && memory_match(args->path + offset, "../", 3))
        {

            struct service_channel *channel;
            unsigned int i;

            for (i = 0; i < CONTAINER_MOUNTS; i++)
            {

                struct container_mount *mount = &container->mounts[i];

                if (session->channel == mount->child.channel && session->id == mount->child.id)
                {

                    session->channel = mount->parent.channel;
                    session->id = mount->parent.id;

                    break;

                }

            }

            channel = getchannel(container, session->channel);
            session->id = channel->protocol->parent(channel->backend, session->id);

            if (!session->id)
                return 0;

        }

        else
        {

            struct service_channel *channel;
            unsigned int i;

            channel = getchannel(container, session->channel);
            session->id = channel->protocol->child(channel->backend, session->id, count, args->path + offset);

            if (!session->id)
                return 0;

            for (i = 0; i < CONTAINER_MOUNTS; i++)
            {

                struct container_mount *mount = &container->mounts[i];

                if (session->channel == mount->parent.channel && session->id == mount->parent.id)
                {

                    session->channel = mount->child.channel;
                    session->id = mount->child.id;

                    break;

                }

            }

        }

    }

    return session->id;

}

static unsigned int create(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int count; char *name;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);
    struct service_channel *channel = getchannel(container, session->channel);

    if (!session->id || !args->count)
        return 0;

    return channel->protocol->create(channel->backend, session->id, args->count, args->name);

}

static unsigned int destroy(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int count; char *name;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);
    struct service_channel *channel = getchannel(container, session->channel);

    if (!session->id || !args->count)
        return 0;

    return channel->protocol->destroy(channel->backend, session->id, args->count, args->name);

}

static unsigned int open(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);
    struct service_channel *channel = getchannel(container, session->channel);

    if (!session->id)
        return 0;

    return session->id = channel->protocol->open(channel->backend, task, args->descriptor, session->id);

}

static unsigned int close(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);
    struct service_channel *channel = getchannel(container, session->channel);

    if (!session->id)
        return 0;

    return session->id = channel->protocol->close(channel->backend, task, args->descriptor, session->id);

}

static unsigned int read(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);
    struct service_channel *channel = getchannel(container, session->channel);
    unsigned int count;

    if (!session->id || !args->count)
        return 0;

    count = channel->protocol->read(channel->backend, task, args->descriptor, session->id, session->offset, args->count, args->buffer);
    session->offset += count;

    return count;

}

static unsigned int seekread(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count; unsigned int offset;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);
    struct service_channel *channel = getchannel(container, session->channel);

    if (!session->id || !args->count)
        return 0;

    return channel->protocol->read(channel->backend, task, args->descriptor, session->id, args->offset, args->count, args->buffer);

}

static unsigned int write(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);
    struct service_channel *channel = getchannel(container, session->channel);
    unsigned int count;

    if (!session->id || !args->count)
        return 0;

    count = channel->protocol->write(channel->backend, task, args->descriptor, session->id, session->offset, args->count, args->buffer);
    session->offset += count;

    return count;

}

static unsigned int seekwrite(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count; unsigned int offset;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);
    struct service_channel *channel = getchannel(container, session->channel);

    if (!session->id || !args->count)
        return 0;

    return channel->protocol->write(channel->backend, task, args->descriptor, session->id, args->offset, args->count, args->buffer);

}

static unsigned int auth(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int channel; unsigned int backend;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);
    struct service_channel *channel = getchannel(container, args->channel);

    if (!session)
        return 0;

    channel->backend = service_findbackend(args->backend);

    if (!channel->backend)
        return 0;

    channel->protocol = service_findprotocol(channel->backend);

    if (!channel->protocol)
        return 0;

    session->channel = args->channel;
    session->id = channel->protocol->root(channel->backend);

    if (!session->id)
        return 0;

    return args->channel;

}

static unsigned int mount(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int mount; unsigned int pdescriptor; unsigned int cdescriptor;} *args = stack;
    struct container_mount *mount = getmount(container, args->mount);
    struct container_session *psession = getsession(container, task, args->pdescriptor);
    struct container_session *csession = getsession(container, task, args->cdescriptor);

    if (!csession->id || !psession->id)
        return 0;

    mount->parent.channel = psession->channel;
    mount->parent.id = psession->id;
    mount->child.channel = csession->channel;
    mount->child.id = csession->id;

    return 1;

}

static unsigned int load(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);
    struct service_channel *channel = getchannel(container, session->channel);
    struct binary_format *format;
    unsigned long physical;
    void (*module_init)(void);
    void (*module_register)(void);

    if (!session->id || !channel->protocol->getphysical)
        return 0;

    /* Physical should be replaced with known address later on */
    physical = channel->protocol->getphysical(channel->backend, session->id);

    if (!physical)
        return 0;

    format = binary_findformat(channel, task, args->descriptor, session->id);

    if (!format || !format->relocate(channel, task, args->descriptor, session->id, physical))
        return 0;

    module_init = (void (*)(void))(format->findsymbol(channel, task, args->descriptor, session->id, 11, "module_init"));

    if (module_init)
        module_init();

    module_register = (void (*)(void))(format->findsymbol(channel, task, args->descriptor, session->id, 15, "module_register"));

    if (module_register)
        module_register();

    return 1;

}

static unsigned int unload(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);
    struct service_channel *channel = getchannel(container, session->channel);
    struct binary_format *format;
    void (*module_unregister)(void);

    if (!session->id)
        return 0;

    format = binary_findformat(channel, task, args->descriptor, session->id);

    if (!format)
        return 0;

    module_unregister = (void (*)(void))(format->findsymbol(channel, task, args->descriptor, session->id, 17, "module_unregister"));

    if (module_unregister)
        module_unregister();

    return 1;

}

static unsigned int seek(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int offset;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);

    if (!session->id)
        return 0;

    return session->offset = args->offset;

}

static unsigned int scan(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int index;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);
    struct service_channel *channel = getchannel(container, session->channel);

    if (!session->id)
        return 0;

    return channel->protocol->scan(channel->backend, session->id, args->index);

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

