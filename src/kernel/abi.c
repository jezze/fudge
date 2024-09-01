#include <fudge.h>
#include "resource.h"
#include "debug.h"
#include "binary.h"
#include "mailbox.h"
#include "task.h"
#include "service.h"
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

static unsigned int debug(unsigned int itask, void *stack)
{

    struct {void *caller; char *message;} *args = stack;

    DEBUG_FMT0(DEBUG_INFO, args->message);

    return 0;

}

static unsigned int walk(unsigned int itask, void *stack)
{

    return 0;

}

static unsigned int create(unsigned int itask, void *stack)
{

    return 0;

}

static unsigned int destroy(unsigned int itask, void *stack)
{

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

    return 0;

}

static unsigned int list(unsigned int itask, void *stack)
{

    return 0;

}

static unsigned int read(unsigned int itask, void *stack)
{

    return 0;

}

static unsigned int load(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int address;} *args = stack;
    struct binary_node node;

    node.address = args->address;

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

    return 0;

}

static unsigned int unload(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int address;} *args = stack;
    struct binary_node node;

    node.address = args->address;

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

    struct {void *caller; struct message *message; unsigned int count; void *data;} *args = stack;

    if (checkbuffer(itask, args->message, sizeof (struct message)) && checkbuffer(itask, args->data, args->count))
        return kernel_pick(itask, args->message, args->count, args->data);

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

    return 0;

}

static unsigned int unlink(unsigned int itask, void *stack)
{

    return 0;

}

static unsigned int announce(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int ichannel;} *args = stack;

    kernel_announce(args->ichannel, itask, 0, 0);

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
    abi_setcallback(0x08, announce);
    abi_setcallback(0x09, debug);
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

