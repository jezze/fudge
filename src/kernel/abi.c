#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "service.h"
#include "container.h"
#include "task.h"

#define CALLS                           16

static unsigned int (*calls[CALLS])(struct container *container, struct task *task, void *stack);

static struct container_server *getserver(struct container *container, unsigned int server)
{

    return &container->servers[server & (CONTAINER_SERVERS - 1)];

}

static struct container_mount *getmount(struct container *container, unsigned int mount)
{

    return &container->mounts[mount & (CONTAINER_MOUNTS - 1)];

}

static struct task_descriptor *getdescriptor(struct task *task, unsigned int descriptor)
{

    return &task->descriptors[descriptor & (TASK_DESCRIPTORS - 1)];

}

static unsigned int walk(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int pdescriptor; char *path; unsigned int length;} *args = stack;
    struct task_descriptor *descriptor = getdescriptor(task, args->descriptor);
    struct task_descriptor *pdescriptor = getdescriptor(task, args->pdescriptor);
    unsigned int offset;
    unsigned int length;

    if (!pdescriptor->server)
        return 0;

    descriptor->server = pdescriptor->server;
    descriptor->state.id = pdescriptor->state.id;

    if (!args->length)
        return 1;

    for (offset = 0; (length = memory_findbyte(args->path + offset, args->length - offset, '/')); offset += length)
    {

        if (length == 3 && memory_match(args->path + offset, "../", 3))
        {

            unsigned int i;

            for (i = 0; i < CONTAINER_MOUNTS; i++)
            {

                struct container_mount *mount = &container->mounts[i];

                if (descriptor->server == mount->child.server && descriptor->state.id == mount->child.id)
                {

                    descriptor->server = mount->parent.server;
                    descriptor->state.id = mount->parent.id;

                    break;

                }

            }

            if (!descriptor->server->protocol->parent(descriptor->server->backend, &descriptor->state))
                return 0;

        }

        else
        {

            unsigned int i;

            if (!descriptor->server->protocol->child(descriptor->server->backend, &descriptor->state, args->path + offset, length))
                return 0;

            for (i = 0; i < CONTAINER_MOUNTS; i++)
            {

                struct container_mount *mount = &container->mounts[i];

                if (descriptor->server == mount->parent.server && descriptor->state.id == mount->parent.id)
                {

                    descriptor->server = mount->child.server;
                    descriptor->state.id = mount->child.id;

                    break;

                }

            }

        }

    }

    return 1;

}

static unsigned int create(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; char *name; unsigned int length;} *args = stack;
    struct task_descriptor *descriptor = getdescriptor(task, args->descriptor);

    if (!args->length || !args->name)
        return 0;

    return descriptor->server->protocol->create(descriptor->server->backend, &descriptor->state, args->name, args->length);

}

static unsigned int destroy(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; char *name; unsigned int length;} *args = stack;
    struct task_descriptor *descriptor = getdescriptor(task, args->descriptor);

    if (!args->length || !args->name)
        return 0;

    return descriptor->server->protocol->destroy(descriptor->server->backend, &descriptor->state, args->name, args->length);

}

static unsigned int open(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct task_descriptor *descriptor = getdescriptor(task, args->descriptor);

    descriptor->state.offset = 0;
    descriptor->state.current = descriptor->server->protocol->step(descriptor->server->backend, descriptor->state.id, 0);

    return descriptor->server->protocol->open(descriptor->server->backend, &descriptor->state);

}

static unsigned int close(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct task_descriptor *descriptor = getdescriptor(task, args->descriptor);

    return descriptor->server->protocol->close(descriptor->server->backend, &descriptor->state);

}

static unsigned int read(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count;} *args = stack;
    struct task_descriptor *descriptor = getdescriptor(task, args->descriptor);

    if (!args->buffer || !args->count)
        return 0;

    descriptor->state.count = descriptor->server->protocol->read(descriptor->server->backend, &descriptor->state, args->buffer, args->count);
    descriptor->state.offset = descriptor->server->protocol->seek(descriptor->server->backend, descriptor->state.offset + descriptor->state.count);
    descriptor->state.current = descriptor->server->protocol->step(descriptor->server->backend, descriptor->state.id, descriptor->state.current);

    return descriptor->state.count;

}

static unsigned int write(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count;} *args = stack;
    struct task_descriptor *descriptor = getdescriptor(task, args->descriptor);

    if (!args->buffer || !args->count)
        return 0;

    descriptor->state.count = descriptor->server->protocol->write(descriptor->server->backend, &descriptor->state, args->buffer, args->count);
    descriptor->state.offset = descriptor->server->protocol->seek(descriptor->server->backend, descriptor->state.offset + descriptor->state.count);
    descriptor->state.current = descriptor->server->protocol->step(descriptor->server->backend, descriptor->state.id, descriptor->state.current);

    return descriptor->state.count;

}

static unsigned int auth(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int backend;} *args = stack;
    struct container_server *server = getserver(container, container->nservers + 1);
    struct task_descriptor *descriptor = getdescriptor(task, args->descriptor);

    server->backend = service_findbackend(args->backend);

    if (!server->backend)
        return 0;

    server->protocol = service_findprotocol(server->backend);

    if (!server->protocol)
        return 0;

    descriptor->server = server;
    descriptor->state.id = descriptor->server->protocol->root(descriptor->server->backend);

    return ++container->nservers;

}

static unsigned int mount(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int pdescriptor; unsigned int cdescriptor;} *args = stack;
    struct container_mount *mount = getmount(container, container->nmounts + 1);
    struct task_descriptor *pdescriptor = getdescriptor(task, args->pdescriptor);
    struct task_descriptor *cdescriptor = getdescriptor(task, args->cdescriptor);

    mount->parent.server = pdescriptor->server;
    mount->parent.id = pdescriptor->state.id;
    mount->child.server = cdescriptor->server;
    mount->child.id = cdescriptor->state.id;

    return ++container->nmounts;

}

static unsigned int load(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct task_descriptor *descriptor = getdescriptor(task, args->descriptor);
    struct binary_format *format;
    void (*module_init)(void);
    void (*module_register)(void);

    if (!descriptor->server->protocol->map(descriptor->server->backend, descriptor->state.id, &descriptor->node))
        return 0;

    format = binary_findformat(&descriptor->node);

    if (!format)
        return 0;

    if (!format->relocate(&descriptor->node))
        return 0;

    module_init = (void (*)(void))(format->findsymbol(&descriptor->node, 11, "module_init"));

    if (module_init)
        module_init();

    module_register = (void (*)(void))(format->findsymbol(&descriptor->node, 15, "module_register"));

    if (module_register)
        module_register();

    return 1;

}

static unsigned int unload(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct task_descriptor *descriptor = getdescriptor(task, args->descriptor);
    struct binary_format *format;
    void (*module_unregister)(void);

    if (!descriptor->server->protocol->map(descriptor->server->backend, descriptor->state.id, &descriptor->node))
        return 0;

    format = binary_findformat(&descriptor->node);

    if (!format)
        return 0;

    module_unregister = (void (*)(void))(format->findsymbol(&descriptor->node, 17, "module_unregister"));

    if (module_unregister)
        module_unregister();

    return 1;

}

static unsigned int seek(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int offset;} *args = stack;
    struct task_descriptor *descriptor = getdescriptor(task, args->descriptor);

    return descriptor->state.offset = descriptor->server->protocol->seek(descriptor->server->backend, args->offset);

}

unsigned int abi_call(unsigned int index, struct container *container, struct task *task, void *stack)
{

    unsigned int (*call)(struct container *container, struct task *task, void *stack) = calls[index & (CALLS - 1)];

    return call ? call(container, task, stack) : 0;

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

}

