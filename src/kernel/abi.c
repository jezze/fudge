#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"
#include "service.h"
#include "container.h"

#define CALLS                           16

static unsigned int (*calls[CALLS])(struct container *container, struct task *task, void *stack);

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

    if (!psession->state.id)
        return 0;

    session->backend = psession->backend;
    session->protocol = psession->protocol;
    session->state.id = psession->state.id;

    for (offset = 0; (count = memory_findbyte(args->path + offset, args->count - offset, '/')); offset += count)
    {

        if (count == 3 && memory_match(args->path + offset, "../", 3))
        {

            unsigned int i;

            for (i = 0; i < CONTAINER_MOUNTS; i++)
            {

                struct container_mount *mount = &container->mounts[i];

                if (session->backend == mount->child.backend && session->state.id == mount->child.id)
                {

                    session->backend = mount->parent.backend;
                    session->protocol = mount->parent.protocol;
                    session->state.id = mount->parent.id;

                    break;

                }

            }

            session->state.id = session->protocol->parent(session->backend, session->state.id);

            if (!session->state.id)
                return 0;

        }

        else
        {

            unsigned int i;

            session->state.id = session->protocol->child(session->backend, session->state.id, count, args->path + offset);

            if (!session->state.id)
                return 0;

            for (i = 0; i < CONTAINER_MOUNTS; i++)
            {

                struct container_mount *mount = &container->mounts[i];

                if (session->backend == mount->parent.backend && session->state.id == mount->parent.id)
                {

                    session->backend = mount->child.backend;
                    session->protocol = mount->child.protocol;
                    session->state.id = mount->child.id;

                    break;

                }

            }

        }

    }

    return session->state.id;

}

static unsigned int create(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int count; char *name;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);

    if (!session->state.id || !args->count)
        return 0;

    return session->protocol->create(session->backend, session->state.id, args->count, args->name);

}

static unsigned int destroy(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int count; char *name;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);

    if (!session->state.id || !args->count)
        return 0;

    return session->protocol->destroy(session->backend, session->state.id, args->count, args->name);

}

static unsigned int open(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);

    if (!session->state.id)
        return 0;

    return session->protocol->open(session->backend, &task->links[args->descriptor], &session->state);

}

static unsigned int close(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);

    if (!session->state.id)
        return 0;

    return session->protocol->close(session->backend, &task->links[args->descriptor], &session->state);

}

static unsigned int read(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);

    if (!session->state.id || !args->count)
        return 0;

    return session->protocol->read(session->backend, &task->links[args->descriptor], &session->state, args->count, args->buffer);

}

static unsigned int write(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);

    if (!session->state.id || !args->count)
        return 0;

    return session->protocol->write(session->backend, &task->links[args->descriptor], &session->state, args->count, args->buffer);

}

static unsigned int auth(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int backend;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);

    session->backend = service_findbackend(args->backend);

    if (!session->backend)
        return 0;

    session->protocol = service_findprotocol(session->backend);

    if (!session->protocol)
        return 0;

    session->state.id = session->protocol->root(session->backend);

    if (!session->state.id)
        return 0;

    return 1;

}

static unsigned int mount(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int mount; unsigned int pdescriptor; unsigned int cdescriptor;} *args = stack;
    struct container_mount *mount = getmount(container, args->mount);
    struct container_session *psession = getsession(container, task, args->pdescriptor);
    struct container_session *csession = getsession(container, task, args->cdescriptor);

    if (!csession->state.id || !psession->state.id)
        return 0;

    mount->parent.backend = psession->backend;
    mount->parent.protocol = psession->protocol;
    mount->parent.id = psession->state.id;
    mount->child.backend = csession->backend;
    mount->child.protocol = csession->protocol;
    mount->child.id = csession->state.id;

    return 1;

}

static unsigned int load(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);
    struct binary_format *format;
    void (*module_init)(void);
    void (*module_register)(void);

    if (!session->state.id)
        return 0;

    if (!session->protocol->map(session->backend, session->state.id, &session->node))
        return 0;

    format = binary_findformat(&session->node);

    if (!format)
        return 0;

    if (!format->relocate(&session->node))
        return 0;

    module_init = (void (*)(void))(format->findsymbol(&session->node, 11, "module_init"));

    if (module_init)
        module_init();

    module_register = (void (*)(void))(format->findsymbol(&session->node, 15, "module_register"));

    if (module_register)
        module_register();

    return 1;

}

static unsigned int unload(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);
    struct binary_format *format;
    void (*module_unregister)(void);

    if (!session->state.id)
        return 0;

    if (!session->protocol->map(session->backend, session->state.id, &session->node))
        return 0;

    format = binary_findformat(&session->node);

    if (!format)
        return 0;

    module_unregister = (void (*)(void))(format->findsymbol(&session->node, 17, "module_unregister"));

    if (module_unregister)
        module_unregister();

    return 1;

}

static unsigned int seek(struct container *container, struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int offset;} *args = stack;
    struct container_session *session = getsession(container, task, args->descriptor);

    if (!session->state.id)
        return 0;

    return session->protocol->seek(session->backend, &session->state, args->offset);

}

unsigned int abi_call(unsigned int index, struct container *container, struct task *task, void *stack)
{

    return calls[index & 0x0F](container, task, stack);

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

