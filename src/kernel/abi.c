#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"
#include "service.h"
#include "kernel.h"

#define CALLS                           16

static unsigned int (*calls[CALLS])(struct task *task, void *stack);

static unsigned int debug(struct task *task, void *stack)
{

    return 0;

}

static void walkmount(struct service *service, unsigned int parent)
{

    struct service_node *node = kernel_findnode(service->server, service->state.id, parent);

    if (node)
    {

        service->server = node->server;
        service->state.id = node->id;

    }

}

static unsigned int walknext(char *path, unsigned int length)
{

    unsigned int i;

    if (!length)
        return 0;

    for (i = 1; i < length && path[i - 1] != '/'; i++);

    return i;

}

static unsigned int walk(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int pdescriptor; char *path; unsigned int length;} *args = stack;
    struct service *service = kernel_getservice(task, args->descriptor);
    struct service *pservice = kernel_getservice(task, args->pdescriptor);
    unsigned int offset;
    unsigned int length;

    if (!pservice->server)
        return 0;

    service->server = pservice->server;
    service->state.id = pservice->state.id;

    if (!args->length)
        return 1;

    for (offset = 0; (length = walknext(args->path + offset, args->length - offset)); offset += length)
    {

        if (length == 2 && args->path[offset] == '.' && args->path[offset + 1] == '.')
        {

            walkmount(service, 1);

            if (!service->server->protocol->parent(service->server->backend, &service->state, service->state.id))
                return 0;

        }

        else
        {

            if (!service->server->protocol->child(service->server->backend, &service->state, service->state.id, args->path + offset, length))
                return 0;

            walkmount(service, 0);

        }

    }

    return 1;

}

static unsigned int create(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; char *name; unsigned int length;} *args = stack;
    struct service *service = kernel_getservice(task, args->descriptor);

    if (!args->length || !args->name)
        return 0;

    return service->server->protocol->create(service->server->backend, &service->state, service->state.id, args->name, args->length);

}

static unsigned int destroy(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; char *name; unsigned int length;} *args = stack;
    struct service *service = kernel_getservice(task, args->descriptor);

    if (!args->length || !args->name)
        return 0;

    return service->server->protocol->destroy(service->server->backend, &service->state, service->state.id, args->name, args->length);

}

static unsigned int open(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct service *service = kernel_getservice(task, args->descriptor);

    service->state.id = service->server->protocol->open(service->server->backend, &service->state, service->state.id);
    service->state.current = service->server->protocol->step(service->server->backend, &service->state, service->state.id, service->state.id);
    service->state.offset = service->server->protocol->seek(service->server->backend, &service->state, service->state.id, 0);

    return service->state.id;

}

static unsigned int close(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct service *service = kernel_getservice(task, args->descriptor);

    service->state.id = service->server->protocol->close(service->server->backend, &service->state, service->state.id);
    service->state.current = service->server->protocol->step(service->server->backend, &service->state, service->state.id, service->state.id);
    service->state.offset = service->server->protocol->seek(service->server->backend, &service->state, service->state.id, 0);

    return service->state.id;

}

static unsigned int step(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct service *service = kernel_getservice(task, args->descriptor);

    service->state.current = service->server->protocol->step(service->server->backend, &service->state, service->state.id, service->state.current);
    service->state.offset = service->server->protocol->seek(service->server->backend, &service->state, service->state.id, 0);

    return service->state.current;

}

static unsigned int read(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count;} *args = stack;
    struct service *service = kernel_getservice(task, args->descriptor);
    unsigned int count;

    if (!args->buffer || !args->count)
        return 0;

    count = service->server->protocol->read(service->server->backend, &service->state, service->state.id, args->buffer, args->count, service->state.offset);
    service->state.offset = service->server->protocol->seek(service->server->backend, &service->state, service->state.id, service->state.offset + count);

    return count;

}

static unsigned int write(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count;} *args = stack;
    struct service *service = kernel_getservice(task, args->descriptor);
    unsigned int count;

    if (!args->buffer || !args->count)
        return 0;

    count = service->server->protocol->write(service->server->backend, &service->state, service->state.id, args->buffer, args->count, service->state.offset);
    service->state.offset = service->server->protocol->seek(service->server->backend, &service->state, service->state.id, service->state.offset + count);

    return count;

}

static unsigned int seek(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int offset;} *args = stack;
    struct service *service = kernel_getservice(task, args->descriptor);

    return service->state.offset = service->server->protocol->seek(service->server->backend, &service->state, service->state.id, args->offset);

}

static unsigned int auth(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int backend; unsigned int protocol;} *args = stack;
    struct service *service = kernel_getservice(task, args->descriptor);
    struct service_server *server = kernel_getfreeserver();

    if (!server)
        return 0;

    server->backend = service_findbackend(args->backend);

    if (!server->backend)
        return 0;

    server->protocol = service_findprotocol(args->protocol);

    if (!server->protocol)
        return 0;

    if (!server->protocol->match(server->backend))
        return 0;

    server->root = server->protocol->root(server->backend);

    if (!server->root)
        return 0;

    service->server = server;
    service->state.id = server->root;

    kernel_useserver(server);

    return 1;

}

static unsigned int mount(struct task *task, void *stack)
{

    struct {void *caller; unsigned int pdescriptor; unsigned int cdescriptor;} *args = stack;
    struct service *pservice = kernel_getservice(task, args->pdescriptor);
    struct service *cservice = kernel_getservice(task, args->cdescriptor);
    struct service_mount *mount = kernel_getfreemount();

    if (!mount)
        return 0;

    mount->parent.server = pservice->server;
    mount->parent.id = pservice->state.id;
    mount->child.server = cservice->server;
    mount->child.id = cservice->state.id;

    kernel_usemount(mount);

    return 1;

}

static unsigned int load(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct service *service = kernel_getservice(task, args->descriptor);
    struct binary_format *format;
    struct binary_node node;
    void (*module_init)(void);
    void (*module_register)(void);

    node.physical = service->server->protocol->map(service->server->backend, &service->state, service->state.id);

    if (!node.physical)
        return 0;

    format = binary_findformat(&node);

    if (!format)
        return 0;

    if (!format->relocate(&node))
        return 0;

    module_init = (void (*)(void))(format->findsymbol(&node, 11, "module_init"));

    if (module_init)
        module_init();

    module_register = (void (*)(void))(format->findsymbol(&node, 15, "module_register"));

    if (module_register)
        module_register();

    return 1;

}

static unsigned int unload(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct service *service = kernel_getservice(task, args->descriptor);
    struct binary_format *format;
    struct binary_node node;
    void (*module_unregister)(void);

    node.physical = service->server->protocol->map(service->server->backend, &service->state, service->state.id);

    if (!node.physical)
        return 0;

    format = binary_findformat(&node);

    if (!format)
        return 0;

    module_unregister = (void (*)(void))(format->findsymbol(&node, 17, "module_unregister"));

    if (module_unregister)
        module_unregister();

    return 1;

}

unsigned int abi_call(unsigned int index, struct task *task, void *stack)
{

    return calls[index & (CALLS - 1)](task, stack);

}

void abi_setup(unsigned int (*spawn)(struct task *task, void *stack), unsigned int (*despawn)(struct task *task, void *stack))
{

    calls[0x00] = debug;
    calls[0x01] = walk;
    calls[0x02] = create;
    calls[0x03] = destroy;
    calls[0x04] = open;
    calls[0x05] = close;
    calls[0x06] = step;
    calls[0x07] = read;
    calls[0x08] = write;
    calls[0x09] = seek;
    calls[0x0A] = auth;
    calls[0x0B] = mount;
    calls[0x0C] = load;
    calls[0x0D] = unload;
    calls[0x0E] = spawn;
    calls[0x0F] = despawn;

}

