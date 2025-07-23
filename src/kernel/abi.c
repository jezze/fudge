#include <fudge.h>
#include "resource.h"
#include "debug.h"
#include "binary.h"
#include "mailbox.h"
#include "task.h"
#include "service.h"
#include "kernel.h"

#define CALLS                           16

static unsigned int (*calls[CALLS])(unsigned int itask, void *stack);

static unsigned int checkuserstack(void *address, unsigned int count)
{

    unsigned int value = (unsigned int)address;

    return (value >= (TASK_STACKVIRTUAL - TASK_STACKSIZE)) && ((value + count) < TASK_STACKVIRTUAL);

}

static unsigned int checkuserspace(unsigned int itask, void *address, unsigned int count)
{

    return checkuserstack(address, count) || kernel_codebase(itask, (unsigned int)address);

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

static unsigned int find(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int namehash; unsigned int index; unsigned int inode;} *args = stack;

    return kernel_findinode(args->namehash, args->index);

}

static unsigned int load(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int address;} *args = stack;

    if (args->address)
    {

        struct binary_format *format = binary_findformat(args->address);

        if (format)
        {

            void (*module_init)(void);
            void (*module_register)(void);

            module_init = (void (*)(void))(format->findsymbol(args->address, 11, "module_init"));

            if (module_init)
                module_init();

            module_register = (void (*)(void))(format->findsymbol(args->address, 15, "module_register"));

            if (module_register)
                module_register();

            return args->address;

        }

    }

    DEBUG_FMT0(DEBUG_ERROR, "load failed");

    return 0;

}

static unsigned int unload(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int address;} *args = stack;

    if (args->address)
    {

        struct binary_format *format = binary_findformat(args->address);

        if (format)
        {

            void (*module_unregister)(void);

            module_unregister = (void (*)(void))(format->findsymbol(args->address, 17, "module_unregister"));

            if (module_unregister)
                module_unregister();

            return args->address;

        }

    }

    DEBUG_FMT0(DEBUG_ERROR, "unload failed");

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

    struct {void *caller; unsigned int ichannel; struct message *message; unsigned int count; void *data;} *args = stack;

    if (checkbuffer(itask, args->message, sizeof (struct message)) && checkbuffer(itask, args->data, args->count))
        return kernel_taskpick(itask, args->ichannel, args->message, args->count, args->data);

    DEBUG_FMT0(DEBUG_ERROR, "pick check failed");

    return 0;

}

static unsigned int place(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int ichannel; unsigned int target; unsigned int event; unsigned int count; void *data;} *args = stack;

    if (checkzerobuffer(itask, args->data, args->count))
        return kernel_taskplace(itask, args->ichannel, args->target, args->event, args->count, args->data);

    DEBUG_FMT0(DEBUG_ERROR, "place check failed");

    return 0;

}

static unsigned int announce(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int ichannel; unsigned int namehash;} *args = stack;

    return kernel_taskannounce(itask, args->ichannel, args->namehash);

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
    abi_setcallback(0x03, spawn);
    abi_setcallback(0x04, despawn);
    abi_setcallback(0x05, kill);
    abi_setcallback(0x06, find);
    abi_setcallback(0x07, load);
    abi_setcallback(0x08, unload);
    abi_setcallback(0x09, announce);
    abi_setcallback(0x0A, debug);
    abi_setcallback(0x0B, debug);
    abi_setcallback(0x0C, debug);
    abi_setcallback(0x0D, debug);
    abi_setcallback(0x0E, debug);
    abi_setcallback(0x0F, debug);

}

