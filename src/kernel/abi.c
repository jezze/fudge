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

static unsigned int checkuserstack(struct task *task, void *address, unsigned int count)
{

    unsigned int value = (unsigned int)address;

    return value >= 0x80000000 - 0x8000 && (value + count) < 0x80000000;

}

static unsigned int checkuserbase(struct task *task, void *address, unsigned int count)
{

    unsigned int value = (unsigned int)address;

    return task->format->findbase(&task->node, value);

}

static unsigned int checkuserspace(struct task *task, void *address, unsigned int count)
{

    return checkuserstack(task, address, count) || checkuserbase(task, address, count);

}

static unsigned int checkbuffer(struct task *task, void *address, unsigned int count)
{

    return (address && count) ? checkuserspace(task, address, count) : 0;

}

static unsigned int checkzerobuffer(struct task *task, void *address, unsigned int count)
{

    return (address && count) ? checkuserspace(task, address, count) : (address == 0 && count == 0);

}

static struct service *findservice(char *path, unsigned int length)
{

    unsigned int offset = buffer_firstbyte(path, length, ':');

    return (offset) ? service_find(offset - 1, path) : 0;

}

static unsigned int findpath(struct service *service, unsigned int id, char *path, unsigned int length)
{

    unsigned int offset = buffer_firstbyte(path, length, ':');

    while (offset < length)
    {

        char *cp = path + offset;
        unsigned int cl = buffer_findbyte(cp, length - offset, '/');

        if (cl == 0)
        {

            id = service->root();

        }

        else if (cl == 2 && cp[0] == '.' && cp[1] == '.')
        {

            if (id != service->root())
                id = service->parent(id);

        }

        else
        {

            id = service->child(id, cp, cl);

        }

        if (!id)
            return 0;

        offset += cl + 1;

    }

    return id;

}

static unsigned int debug(struct task *task, void *stack)
{

    return 0;

}

static unsigned int walk(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int pdescriptor; char *path; unsigned int length;} *args = stack;
    struct descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);
    struct descriptor *pdescriptor = kernel_getdescriptor(task, args->pdescriptor);
    struct service *service;

    if (!checkzerobuffer(task, args->path, args->length) || !descriptor_check(pdescriptor))
        return 0;

    service = findservice(args->path, args->length);

    descriptor_copy(descriptor, pdescriptor);

    if (service)
    {

        descriptor->service = service;
        descriptor->id = service->root();

    }

    return descriptor->id = findpath(descriptor->service, descriptor->id, args->path, args->length);

}

static unsigned int create(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int pdescriptor; char *name; unsigned int length;} *args = stack;
    struct descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);
    struct descriptor *pdescriptor = kernel_getdescriptor(task, args->pdescriptor);

    if (!checkbuffer(task, args->name, args->length) || !descriptor_check(pdescriptor))
        return 0;

    descriptor_copy(descriptor, pdescriptor);

    return descriptor->id = descriptor->service->create(descriptor->id, args->name, args->length);

}

static unsigned int destroy(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor;} *args = stack;
    struct descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!descriptor_check(descriptor))
        return 0;

    return descriptor->id = descriptor->service->destroy(descriptor->id);

}

static unsigned int kill(struct task *task, void *stack)
{

    struct {void *caller; unsigned int id;} *args = stack;

    kernel_kill(task->id, args->id);

    return 0;

}

static unsigned int stat(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; struct record *record;} *args = stack;
    struct descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!checkbuffer(task, args->record, sizeof (struct record)) || !descriptor_check(descriptor))
        return 0;

    return descriptor->service->stat(descriptor->id, args->record);

}

static unsigned int list(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; unsigned int cdescriptor; unsigned int count; struct record *records;} *args = stack;
    struct descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);
    struct descriptor *cdescriptor = kernel_getdescriptor(task, args->cdescriptor);
    unsigned int count;

    if (!checkbuffer(task, args->records, args->count * sizeof (struct record)) || !descriptor_check(descriptor) || !descriptor_check(cdescriptor))
        return 0;

    if (descriptor->id == cdescriptor->id)
        cdescriptor->id = 0;

    count = descriptor->service->list(descriptor->id, cdescriptor->id, args->count, args->records);
    cdescriptor->id = (count) ? args->records[count - 1].id : 0;

    return count;

}

static unsigned int read(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count; unsigned int offset;} *args = stack;
    struct descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!checkbuffer(task, args->buffer, args->count) || !descriptor_check(descriptor))
        return 0;

    return descriptor->service->read(descriptor->id, args->buffer, args->count, args->offset);

}

static unsigned int write(struct task *task, void *stack)
{

    struct {void *caller; unsigned int descriptor; void *buffer; unsigned int count; unsigned int offset;} *args = stack;
    struct descriptor *descriptor = kernel_getdescriptor(task, args->descriptor);

    if (!checkbuffer(task, args->buffer, args->count) || !descriptor_check(descriptor))
        return 0;

    return descriptor->service->write(descriptor->id, args->buffer, args->count, args->offset);

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

    if (!format || !format->relocate(&node))
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

static unsigned int spawn(struct task *task, void *stack)
{

    return 0;

}

static unsigned int despawn(struct task *task, void *stack)
{

    task_signal(task, TASK_SIGNAL_KILL);

    return 0;

}

static unsigned int pick(struct task *task, void *stack)
{

    struct {void *caller; struct message *message; void *data;} *args = stack;
    unsigned int count;

    if (!checkbuffer(task, args->message, sizeof (struct message)) || !checkbuffer(task, args->data, MESSAGE_SIZE))
        return 0;

    count = kernel_pick(task->id, args->message, args->data);

    if (!count)
        task_signal(task, TASK_SIGNAL_BLOCK);

    return count;

}

static unsigned int place(struct task *task, void *stack)
{

    struct {void *caller; unsigned int id; struct message *message; void *data;} *args = stack;

    if (!checkbuffer(task, args->message, sizeof (struct message)) || !checkzerobuffer(task, args->data, message_datasize(args->message)))
        return 0;

    return kernel_place(task->id, args->id, args->message, args->data);

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

    if (!checkzerobuffer(task, args->data, args->count) || !descriptor_check(descriptor))
        return 0;

    return descriptor->service->notify(descriptor->id, task->id, args->event, args->count, args->data);

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
    abi_setcallback(0x04, kill);
    abi_setcallback(0x05, stat);
    abi_setcallback(0x06, list);
    abi_setcallback(0x07, read);
    abi_setcallback(0x08, write);
    abi_setcallback(0x09, notify);
    abi_setcallback(0x0A, load);
    abi_setcallback(0x0B, unload);
    abi_setcallback(0x0C, spawn);
    abi_setcallback(0x0D, despawn);
    abi_setcallback(0x0E, pick);
    abi_setcallback(0x0F, place);
    abi_setcallback(0x10, link);
    abi_setcallback(0x11, unlink);
    abi_setcallback(0x12, debug);
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

