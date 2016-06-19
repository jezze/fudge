#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "service.h"
#include "task.h"
#include "container.h"

#define CALLS                           16

static unsigned int (*calls[CALLS])(struct container *container, struct task *task, void *stack);

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

    struct {void *caller; unsigned int descriptor; unsigned int pdescriptor; unsigned int count; char *path;} *args = stack;
    struct task_descriptor *descriptor = getdescriptor(task, args->descriptor);
    struct task_descriptor *pdescriptor = getdescriptor(task, args->pdescriptor);
    unsigned int offset;
    unsigned int count;

    if (!pdescriptor->backend || !pdescriptor->protocol)
        return 0;

    descriptor->backend = pdescriptor->backend;
    descriptor->protocol = pdescriptor->protocol;
    descriptor->state.id = pdescriptor->state.id;

    if (!args->count)
        return 1;

    for (offset = 0; (count = memory_findbyte(args->path + offset, args->count - offset, '/')); offset += count)
    {

        if (count == 3 && memory_match(args->path + offset, "../", 3))
        {

            unsigned int i;

            for (i = 0; i < CONTAINER_MOUNTS; i++)
            {

                struct container_mount *mount = &container->mounts[i];

                if (descriptor->backend == mount->child.backend && descriptor->protocol == mount->child.protocol && descriptor->state.id == mount->child.id)
                {

                    descriptor->backend = mount->parent.backend;
                    descriptor->protocol = mount->parent.protocol;
                    descriptor->state.id = mount->parent.id;

                    break;

                }

            }

            if (!descriptor->protocol->parent(descriptor->backend, &descriptor->state))
                return 0;

        }

        else
        {

            unsigned int i;

            if (!descriptor->protocol->child(descriptor->backend, &descriptor->state, count, args->path + offset))
                return 0;

            for (i = 0; i < CONTAINER_MOUNTS; i++)
            {

                struct container_mount *mount = &container->mounts[i];

                if (descriptor->backend == mount->parent.backend && descriptor->protocol == mount->parent.protocol && descriptor->state.id == mount->parent.id)
                {

                    descriptor->backend = mount->child.backend;
                    descriptor->protocol = mount->child.protocol;
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

    struct {void *caller; unsigned int descriptor; unsigned int count; char *name;} *args = stack;
    struct task_descriptor *descriptor = getdescriptor(task, args->descriptor);

    if (!args->count || !args->name)
        return 0;

    return descriptor->protocol->create(descriptor->backend, &descriptor->state, args->count, args->name);

}

static unsigned int destroy(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int count; char *name;} *args = stack;
    struct task_descriptor *descriptor = getdescriptor(task, args->descriptor);

    if (!args->count || !args->name)
        return 0;

    return descriptor->protocol->destroy(descriptor->backend, &descriptor->state, args->count, args->name);

}

static unsigned int open(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct task_descriptor *descriptor = getdescriptor(task, args->descriptor);

    return descriptor->protocol->open(descriptor->backend, &descriptor->state);

}

static unsigned int close(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct task_descriptor *descriptor = getdescriptor(task, args->descriptor);

    return descriptor->protocol->close(descriptor->backend, &descriptor->state);

}

static unsigned int read(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count;} *args = stack;
    struct task_descriptor *descriptor = getdescriptor(task, args->descriptor);

    if (!args->buffer || !args->count)
        return 0;

    return descriptor->protocol->read(descriptor->backend, &descriptor->state, args->count, args->buffer);

}

static unsigned int write(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count;} *args = stack;
    struct task_descriptor *descriptor = getdescriptor(task, args->descriptor);

    if (!args->buffer || !args->count)
        return 0;

    return descriptor->protocol->write(descriptor->backend, &descriptor->state, args->count, args->buffer);

}

static unsigned int auth(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int backend;} *args = stack;
    struct task_descriptor *descriptor = getdescriptor(task, args->descriptor);

    descriptor->backend = service_findbackend(args->backend);

    if (!descriptor->backend)
        return 0;

    descriptor->protocol = service_findprotocol(descriptor->backend);

    if (!descriptor->protocol)
        return 0;

    descriptor->state.id = descriptor->protocol->root(descriptor->backend);

    return 1;

}

static unsigned int mount(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int mount; unsigned int pdescriptor; unsigned int cdescriptor;} *args = stack;
    struct container_mount *mount = getmount(container, args->mount);
    struct task_descriptor *pdescriptor = getdescriptor(task, args->pdescriptor);
    struct task_descriptor *cdescriptor = getdescriptor(task, args->cdescriptor);

    mount->parent.backend = pdescriptor->backend;
    mount->parent.protocol = pdescriptor->protocol;
    mount->parent.id = pdescriptor->state.id;
    mount->child.backend = cdescriptor->backend;
    mount->child.protocol = cdescriptor->protocol;
    mount->child.id = cdescriptor->state.id;

    return 1;

}

static unsigned int load(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct task_descriptor *descriptor = getdescriptor(task, args->descriptor);
    struct binary_format *format;
    void (*module_init)(void);
    void (*module_register)(void);

    if (!descriptor->protocol->map(descriptor->backend, &descriptor->state, &descriptor->node))
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

    if (!descriptor->protocol->map(descriptor->backend, &descriptor->state, &descriptor->node))
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

    return descriptor->protocol->seek(descriptor->backend, &descriptor->state, args->offset);

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

