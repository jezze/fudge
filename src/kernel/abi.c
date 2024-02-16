#include <fudge.h>
#include "resource.h"
#include "debug.h"
#include "binary.h"
#include "task.h"
#include "service.h"
#include "descriptor.h"
#include "kernel.h"

#define CALLS                           32

static unsigned int (*calls[CALLS])(unsigned int itask, void *stack);

static unsigned int checkuserspace(unsigned int itask, void *address, unsigned int count)
{

    unsigned int value = (unsigned int)address;

    return ((((value >= (0x80000000 - 0x8000)) && ((value + count) < 0x80000000))) || kernel_codebase(itask, value));

}

static unsigned int checkbuffer(unsigned int itask, void *address, unsigned int count)
{

    return (address && count) ? checkuserspace(itask, address, count) : 0;

}

static unsigned int checkzerobuffer(unsigned int itask, void *address, unsigned int count)
{

    return (address && count) ? checkuserspace(itask, address, count) : (address == 0 && count == 0);

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

static unsigned int debug(unsigned int itask, void *stack)
{

    struct {void *caller; char *message;} *args = stack;

    DEBUG_FMT0(DEBUG_INFO, args->message);

    return 0;

}

static unsigned int walk(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int idescriptor; unsigned int pdescriptor; char *path; unsigned int length;} *args = stack;
    struct descriptor *idescriptor = kernel_getdescriptor(itask, args->idescriptor);
    struct descriptor *pdescriptor = kernel_getdescriptor(itask, args->pdescriptor);
    struct service *service;

    if (checkzerobuffer(itask, args->path, args->length) && descriptor_check(pdescriptor))
    {

        kernel_copydescriptor(itask, args->idescriptor, itask, args->pdescriptor);

        if ((service = findservice(args->path, args->length)))
            kernel_setdescriptor(itask, args->idescriptor, service, service->root());

        return idescriptor->id = findpath(idescriptor->service, idescriptor->id, args->path, args->length);

    }

    DEBUG_FMT0(DEBUG_ERROR, "walk check failed");

    return 0;

}

static unsigned int create(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int idescriptor; unsigned int pdescriptor; char *name; unsigned int length;} *args = stack;
    struct descriptor *idescriptor = kernel_getdescriptor(itask, args->idescriptor);
    struct descriptor *pdescriptor = kernel_getdescriptor(itask, args->pdescriptor);

    if (checkbuffer(itask, args->name, args->length) && descriptor_check(pdescriptor))
    {

        kernel_copydescriptor(itask, args->idescriptor, itask, args->pdescriptor);

        return idescriptor->id = idescriptor->service->create(idescriptor->id, args->name, args->length);

    }

    DEBUG_FMT0(DEBUG_ERROR, "create check failed");

    return 0;

}

static unsigned int destroy(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int idescriptor;} *args = stack;
    struct descriptor *idescriptor = kernel_getdescriptor(itask, args->idescriptor);

    if (descriptor_check(idescriptor))
        return idescriptor->id = idescriptor->service->destroy(idescriptor->id);

    DEBUG_FMT0(DEBUG_ERROR, "destroy check failed");

    return 0;

}

static unsigned int kill(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int itask;} *args = stack;

    kernel_signal(args->itask, TASK_SIGNAL_KILL);

    return 0;

}

static unsigned int stat(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int idescriptor; struct record *record;} *args = stack;
    struct descriptor *idescriptor = kernel_getdescriptor(itask, args->idescriptor);

    if (checkbuffer(itask, args->record, sizeof (struct record)) && descriptor_check(idescriptor))
        return idescriptor->service->stat(idescriptor->id, args->record);

    DEBUG_FMT0(DEBUG_ERROR, "stat check failed");

    return 0;

}

static unsigned int list(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int idescriptor; unsigned int cdescriptor; unsigned int count; struct record *records;} *args = stack;
    struct descriptor *idescriptor = kernel_getdescriptor(itask, args->idescriptor);
    struct descriptor *cdescriptor = kernel_getdescriptor(itask, args->cdescriptor);
    unsigned int count;

    if (checkbuffer(itask, args->records, args->count * sizeof (struct record)) && descriptor_check(idescriptor) && descriptor_check(cdescriptor))
    {

        if (idescriptor->id == cdescriptor->id)
            cdescriptor->id = 0;

        count = idescriptor->service->list(idescriptor->id, cdescriptor->id, args->count, args->records);
        cdescriptor->id = (count) ? args->records[count - 1].id : 0;

        return count;

    }

    DEBUG_FMT0(DEBUG_ERROR, "list check failed");

    return 0;

}

static unsigned int read(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int idescriptor; void *buffer; unsigned int count; unsigned int offset;} *args = stack;
    struct descriptor *idescriptor = kernel_getdescriptor(itask, args->idescriptor);

    if (checkbuffer(itask, args->buffer, args->count) && descriptor_check(idescriptor))
        return idescriptor->service->read(idescriptor->id, args->buffer, args->count, args->offset);

    DEBUG_FMT0(DEBUG_ERROR, "read check failed");

    return 0;

}

static unsigned int write(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int idescriptor; void *buffer; unsigned int count; unsigned int offset;} *args = stack;
    struct descriptor *idescriptor = kernel_getdescriptor(itask, args->idescriptor);

    if (checkbuffer(itask, args->buffer, args->count) && descriptor_check(idescriptor))
        return idescriptor->service->write(idescriptor->id, args->buffer, args->count, args->offset);

    DEBUG_FMT0(DEBUG_ERROR, "write check failed");

    return 0;

}

static unsigned int load(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int idescriptor;} *args = stack;
    struct descriptor *idescriptor = kernel_getdescriptor(itask, args->idescriptor);

    if (descriptor_check(idescriptor))
    {

        struct binary_node node;

        node.address = idescriptor->service->map(idescriptor->id, 0);

        if (node.address)
        {

            struct binary_format *format = binary_findformat(&node);

            if (format)
            {

                void (*module_init)(void);
                void (*module_register)(void);

                format->relocate(&node);

                module_init = (void (*)(void))(format->findsymbol(&node, 11, "module_init"));

                if (module_init)
                    module_init();

                module_register = (void (*)(void))(format->findsymbol(&node, 15, "module_register"));

                if (module_register)
                    module_register();

                return node.address;

            }

        }

    }

    return 0;

}

static unsigned int unload(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int idescriptor;} *args = stack;
    struct descriptor *idescriptor = kernel_getdescriptor(itask, args->idescriptor);

    if (descriptor_check(idescriptor))
    {

        struct binary_node node;

        node.address = idescriptor->service->map(idescriptor->id, 0);

        if (node.address)
        {

            struct binary_format *format = binary_findformat(&node);

            if (format)
            {

                void (*module_unregister)(void);

                module_unregister = (void (*)(void))(format->findsymbol(&node, 17, "module_unregister"));

                if (module_unregister)
                    module_unregister();

                return node.address;

            }

        }

    }

    return 0;

}

static unsigned int spawn(unsigned int itask, void *stack)
{

    return 0;

}

static unsigned int despawn(unsigned int itask, void *stack)
{

    kernel_signal(itask, TASK_SIGNAL_KILL);

    return 0;

}

static unsigned int pick(unsigned int itask, void *stack)
{

    struct {void *caller; struct message *message; void *data;} *args = stack;

    if (checkbuffer(itask, args->message, sizeof (struct message)) && checkbuffer(itask, args->data, MESSAGE_SIZE))
        return kernel_pick(itask, args->message, args->data);

    DEBUG_FMT0(DEBUG_ERROR, "pick check failed");

    return 0;

}

static unsigned int place(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int ichannel; unsigned int event; unsigned int count; void *data;} *args = stack;

    if (checkzerobuffer(itask, args->data, args->count))
        return kernel_place(itask, args->ichannel, args->event, args->count, args->data);

    DEBUG_FMT0(DEBUG_ERROR, "place check failed");

    return 0;

}

static unsigned int link(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int idescriptor; unsigned int source;} *args = stack;
    struct descriptor *idescriptor = kernel_getdescriptor(itask, args->idescriptor);

    if (descriptor_check(idescriptor))
        return idescriptor->service->link(idescriptor->id, itask, args->source);

    DEBUG_FMT0(DEBUG_ERROR, "link check failed");

    return 0;

}

static unsigned int unlink(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int idescriptor;} *args = stack;
    struct descriptor *idescriptor = kernel_getdescriptor(itask, args->idescriptor);

    if (descriptor_check(idescriptor))
        return idescriptor->service->unlink(idescriptor->id, itask);

    DEBUG_FMT0(DEBUG_ERROR, "unlink check failed");

    return 0;

}

static unsigned int notify(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int idescriptor; unsigned int event; unsigned int count; void *data;} *args = stack;
    struct descriptor *idescriptor = kernel_getdescriptor(itask, args->idescriptor);

    if (checkzerobuffer(itask, args->data, args->count) && descriptor_check(idescriptor))
        return idescriptor->service->notify(idescriptor->id, itask, args->event, args->count, args->data);

    DEBUG_FMT0(DEBUG_ERROR, "notify check failed");

    return 0;

}

static unsigned int announce(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int ichannel;} *args = stack;

    kernel_announce(itask, args->ichannel);

    return 0;

}

unsigned int abi_call(unsigned int index, unsigned int itask, void *stack)
{

    return index < CALLS ? calls[index](itask, stack) : 0;

}

void abi_setcallback(unsigned int index, unsigned int (*callback)(unsigned int itask, void *stack))
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
    abi_setcallback(0x12, announce);
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

