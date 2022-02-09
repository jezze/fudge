#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"
#include "core.h"
#include "service.h"
#include "descriptor.h"
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
    struct descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);
    struct descriptor *pdescriptor = kernel_getdescriptor(task, args->pdescriptor);
    unsigned int colon = buffer_findbyte(args->path, args->length, ':');
    unsigned int offset = 0;

    if (!descriptor_check(pdescriptor))
        return 0;

    descriptor_copy(descriptor, pdescriptor);

    if (colon < args->length)
    {

        struct service *service = service_find(colon, args->path);

        if (service)
        {

            descriptor->service = service;
            descriptor->id = service->root();
            offset += colon + 1;

        }

    }

    while (offset < args->length)
    {

        char *cp = args->path + offset;
        unsigned int cl = buffer_findbyte(cp, args->length - offset, '/');

        if (cl == 0)
            descriptor->id = descriptor->service->root();
        else if (cl == 2 && cp[0] == '.' && cp[1] == '.')
            descriptor->id = descriptor->service->parent(descriptor->id);
        else
            descriptor->id = descriptor->service->child(descriptor->id, cp, cl);

        if (!descriptor->id)
            return 0;

        offset += cl + 1;

    }

    return descriptor->id;

}

static unsigned int create(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int pdescriptor; char *name; unsigned int length;} *args = stack;
    struct descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);
    struct descriptor *pdescriptor = kernel_getdescriptor(task, args->pdescriptor);

    if (!descriptor_check(pdescriptor))
        return 0;

    if (!args->length || !args->name)
        return 0;

    descriptor_copy(descriptor, pdescriptor);

    descriptor->id = descriptor->service->create(descriptor->id, args->name, args->length);

    return descriptor->id;

}

static unsigned int destroy(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!descriptor_check(descriptor))
        return 0;

    descriptor->id = descriptor->service->destroy(descriptor->id);

    return descriptor->id;

}

static unsigned int reset(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!descriptor_check(descriptor))
        return 0;

    descriptor->current = descriptor->service->step(descriptor->id, 0);
    descriptor->offset = descriptor->service->seek(descriptor->id, 0);

    return descriptor->id;

}

static unsigned int step(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!descriptor_check(descriptor))
        return 0;

    descriptor->current = descriptor->service->step(descriptor->id, descriptor->current);
    descriptor->offset = descriptor->service->seek(descriptor->id, 0);

    return descriptor->current;

}

static unsigned int read(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count;} *args = stack;
    struct descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!descriptor_check(descriptor))
        return 0;

    if (!args->buffer || !args->count)
        return 0;

    descriptor->count = descriptor->service->read(descriptor->id, descriptor->current, args->buffer, args->count, descriptor->offset);
    descriptor->offset = descriptor->service->seek(descriptor->id, descriptor->offset + descriptor->count);

    return descriptor->count;

}

static unsigned int write(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count;} *args = stack;
    struct descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!descriptor_check(descriptor))
        return 0;

    if (!args->buffer || !args->count)
        return 0;

    descriptor->count = descriptor->service->write(descriptor->id, descriptor->current, args->buffer, args->count, descriptor->offset);
    descriptor->offset = descriptor->service->seek(descriptor->id, descriptor->offset + descriptor->count);

    return descriptor->count;

}

static unsigned int seek(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int offset;} *args = stack;
    struct descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!descriptor_check(descriptor))
        return 0;

    descriptor->offset = descriptor->service->seek(descriptor->id, args->offset);

    return descriptor->offset;

}

static unsigned int load(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);
    struct binary_format *format;
    struct binary_node node;
    void (*module_init)(void);
    void (*module_register)(void);

    if (!descriptor_check(descriptor))
        return 0;

    node.address = descriptor->service->map(descriptor->id);

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

static unsigned int spawn(struct task *task, void *stack)
{

    return 0;

}

static unsigned int despawn(struct task *task, void *stack)
{

    task_signal(task, TASK_SIGNAL_KILL);

    return 0;

}

static unsigned int unload(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);
    struct binary_format *format;
    struct binary_node node;
    void (*module_unregister)(void);

    if (!descriptor_check(descriptor))
        return 0;

    node.address = descriptor->service->map(descriptor->id);

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
    struct descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!descriptor_check(descriptor))
        return 0;

    return descriptor->service->link(descriptor->id, task->id);

}

static unsigned int unlink(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!descriptor_check(descriptor))
        return 0;

    return descriptor->service->unlink(descriptor->id, task->id);

}

static unsigned int notify(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int event; unsigned int count; void *data;} *args = stack;
    struct descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!descriptor_check(descriptor))
        return 0;

    return descriptor->service->notify(descriptor->id, task->id, args->event, args->count, args->data);

}

static unsigned int kill(struct task *task, void *stack)
{

    struct {void *caller; unsigned int id;} *args = stack;

    kernel_kill(task->id, args->id);

    return 0;

}

unsigned int abi_call(unsigned int index, struct task *task, void *stack)
{

    return index < CALLS ? calls[index](task, stack) : 0;

}

void abi_setcallback(unsigned int index, unsigned int (*callback)(struct task *task, void *stack))
{

    calls[index] = callback;

}

void abi_setup(void)
{

    abi_setcallback(0x00, debug);
    abi_setcallback(0x01, walk);
    abi_setcallback(0x02, create);
    abi_setcallback(0x03, destroy);
    abi_setcallback(0x04, reset);
    abi_setcallback(0x05, step);
    abi_setcallback(0x06, read);
    abi_setcallback(0x07, write);
    abi_setcallback(0x08, seek);
    abi_setcallback(0x09, load);
    abi_setcallback(0x0A, unload);
    abi_setcallback(0x0B, spawn);
    abi_setcallback(0x0C, despawn);
    abi_setcallback(0x0D, pick);
    abi_setcallback(0x0E, place);
    abi_setcallback(0x0F, link);
    abi_setcallback(0x10, unlink);
    abi_setcallback(0x11, notify);
    abi_setcallback(0x12, kill);
    abi_setcallback(0x13, debug);
    abi_setcallback(0x14, debug);
    abi_setcallback(0x15, debug);
    abi_setcallback(0x16, debug);
    abi_setcallback(0x17, debug);
    abi_setcallback(0x18, debug);
    abi_setcallback(0x19, debug);
    abi_setcallback(0x1A, debug);
    abi_setcallback(0x1B, debug);
    abi_setcallback(0x1C, debug);
    abi_setcallback(0x1D, debug);
    abi_setcallback(0x1E, debug);
    abi_setcallback(0x1F, debug);

}

