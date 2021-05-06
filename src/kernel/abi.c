#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"
#include "core.h"
#include "service.h"
#include "kernel.h"

#define CALLS                           32

static unsigned int (*calls[CALLS])(struct task *task, void *stack);

static unsigned int debug(struct task *task, void *stack)
{

    return 0;

}

static unsigned int walk(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int pdescriptor; char *path; unsigned int length;} *args = stack;
    struct service_descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);
    struct service_descriptor *pdescriptor = kernel_getdescriptor(task, args->pdescriptor);

    if (!pdescriptor->protocol)
        return 0;

    descriptor->protocol = pdescriptor->protocol;
    descriptor->id = pdescriptor->id;

    return kernel_walk(descriptor, args->path, args->length);

}

static unsigned int create(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int pdescriptor; char *name; unsigned int length;} *args = stack;
    struct service_descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);
    struct service_descriptor *pdescriptor = kernel_getdescriptor(task, args->pdescriptor);

    if (!pdescriptor->protocol)
        return 0;

    if (!args->length || !args->name)
        return 0;

    descriptor->protocol = pdescriptor->protocol;
    descriptor->id = pdescriptor->protocol->create(pdescriptor->id, args->name, args->length);

    return descriptor->id;

}

static unsigned int destroy(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct service_descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!descriptor->protocol)
        return 0;

    return descriptor->protocol->destroy(descriptor->id);

}

static unsigned int reset(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct service_descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!descriptor->protocol)
        return 0;

    descriptor->current = descriptor->protocol->step(descriptor->id, descriptor->id);
    descriptor->offset = descriptor->protocol->seek(descriptor->id, 0);

    return descriptor->id;

}

static unsigned int step(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct service_descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!descriptor->protocol)
        return 0;

    descriptor->current = descriptor->protocol->step(descriptor->id, descriptor->current);
    descriptor->offset = descriptor->protocol->seek(descriptor->id, 0);

    return descriptor->current;

}

static unsigned int read(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count;} *args = stack;
    struct service_descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!descriptor->protocol)
        return 0;

    if (!args->buffer || !args->count)
        return 0;

    descriptor->count = descriptor->protocol->read(descriptor->id, descriptor->current, args->buffer, args->count, descriptor->offset);
    descriptor->offset = descriptor->protocol->seek(descriptor->id, descriptor->offset + descriptor->count);

    return descriptor->count;

}

static unsigned int write(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count;} *args = stack;
    struct service_descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!descriptor->protocol)
        return 0;

    if (!args->buffer || !args->count)
        return 0;

    descriptor->count = descriptor->protocol->write(descriptor->id, descriptor->current, args->buffer, args->count, descriptor->offset);
    descriptor->offset = descriptor->protocol->seek(descriptor->id, descriptor->offset + descriptor->count);

    return descriptor->count;

}

static unsigned int seek(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int offset;} *args = stack;
    struct service_descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!descriptor->protocol)
        return 0;

    descriptor->offset = descriptor->protocol->seek(descriptor->id, args->offset);

    return descriptor->offset;

}

static unsigned int load(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct service_descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);
    struct binary_format *format;
    struct binary_node node;
    void (*module_init)(void);
    void (*module_register)(void);

    if (!descriptor->protocol)
        return 0;

    node.address = descriptor->protocol->map(descriptor->id);

    if (!node.address)
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
    struct service_descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);
    struct binary_format *format;
    struct binary_node node;
    void (*module_unregister)(void);

    if (!descriptor->protocol)
        return 0;

    node.address = descriptor->protocol->map(descriptor->id);

    if (!node.address)
        return 0;

    format = binary_findformat(&node);

    if (!format)
        return 0;

    module_unregister = (void (*)(void))(format->findsymbol(&node, 17, "module_unregister"));

    if (module_unregister)
        module_unregister();

    return 1;

}

static unsigned int pick(struct task *task, void *stack)
{

    struct {void *caller; struct message_header *header; void *data;} *args = stack;

    return kernel_pick(task->id, args->header, args->data);

}

static unsigned int place(struct task *task, void *stack)
{

    struct {void *caller; unsigned int id; struct message_header *header; void *data;} *args = stack;

    return kernel_place(task->id, args->id, args->header, args->data);

}

static unsigned int link(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct service_descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!descriptor->protocol)
        return 0;

    return descriptor->protocol->link(descriptor->id, task->id);

}

static unsigned int unlink(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct service_descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!descriptor->protocol)
        return 0;

    return descriptor->protocol->unlink(descriptor->id, task->id);

}

static unsigned int notify(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int event; unsigned int count; void *data;} *args = stack;
    struct service_descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!descriptor->protocol)
        return 0;

    return descriptor->protocol->notify(descriptor->id, task->id, args->event, args->count, args->data);

}

static unsigned int kill(struct task *task, void *stack)
{

    struct {void *caller; unsigned int id;} *args = stack;

    kernel_killtask(args->id);

    return 0;

}

unsigned int abi_call(unsigned int index, struct task *task, void *stack)
{

    return index < CALLS ? calls[index](task, stack) : 0;

}

void abi_setup(unsigned int (*spawn)(struct task *task, void *stack), unsigned int (*despawn)(struct task *task, void *stack))
{

    calls[0x00] = debug;
    calls[0x01] = walk;
    calls[0x02] = create;
    calls[0x03] = destroy;
    calls[0x04] = reset;
    calls[0x05] = step;
    calls[0x06] = read;
    calls[0x07] = write;
    calls[0x08] = seek;
    calls[0x09] = load;
    calls[0x0A] = unload;
    calls[0x0B] = spawn;
    calls[0x0C] = despawn;
    calls[0x0D] = pick;
    calls[0x0E] = place;
    calls[0x0F] = link;
    calls[0x10] = unlink;
    calls[0x11] = notify;
    calls[0x12] = kill;
    calls[0x13] = debug;
    calls[0x14] = debug;
    calls[0x15] = debug;
    calls[0x16] = debug;
    calls[0x17] = debug;
    calls[0x18] = debug;
    calls[0x19] = debug;
    calls[0x1A] = debug;
    calls[0x1B] = debug;
    calls[0x1C] = debug;
    calls[0x1D] = debug;
    calls[0x1E] = debug;
    calls[0x1F] = debug;

}

