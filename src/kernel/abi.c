#include <fudge.h>
#include "resource.h"
#include "debug.h"
#include "binary.h"
#include "mailbox.h"
#include "task.h"
#include "kernel.h"

#define CALLS                           16

static unsigned int (*calls[CALLS])(unsigned int itask, void *stack);

static unsigned int checkuserspace(unsigned int itask, void *address, unsigned int count)
{

    unsigned int value = (unsigned int)address;

    return ((((value >= (TASK_STACKVIRTUAL - TASK_STACKSIZE)) && ((value + count) < TASK_STACKVIRTUAL))) || kernel_codebase(itask, value));

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

static unsigned int kill(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int itask;} *args = stack;

    kernel_signal(args->itask, TASK_SIGNAL_KILL);

    return 0;

}

static unsigned int load(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int address;} *args = stack;
    struct binary_node node;

    binary_initnode(&node, args->address);

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

    binary_initnode(&node, args->address);

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

static unsigned int announce(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int ichannel;} *args = stack;

    kernel_announce(args->ichannel, itask, 0);

    return 0;

}

static unsigned int unannounce(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int ichannel;} *args = stack;

    kernel_unannounce(args->ichannel);

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
    abi_setcallback(0x01, pick);
    abi_setcallback(0x02, place);
    abi_setcallback(0x03, announce);
    abi_setcallback(0x04, unannounce);
    abi_setcallback(0x05, kill);
    abi_setcallback(0x06, debug);
    abi_setcallback(0x07, debug);
    abi_setcallback(0x08, debug);
    abi_setcallback(0x09, debug);
    abi_setcallback(0x0A, load);
    abi_setcallback(0x0B, unload);
    abi_setcallback(0x0C, spawn);
    abi_setcallback(0x0D, despawn);
    abi_setcallback(0x0E, debug);
    abi_setcallback(0x0F, debug);

}

