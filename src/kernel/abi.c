#include <fudge.h>
#include "resource.h"
#include "debug.h"
#include "mmap.h"
#include "binary.h"
#include "service.h"
#include "mailbox.h"
#include "task.h"
#include "pool.h"
#include "kernel.h"

#define CALLS                           16

static unsigned int (*calls[CALLS])(unsigned int itask, void *stack);
static unsigned long (*locate)(unsigned int target, unsigned int id);

static unsigned int checkargs(void *address, unsigned int count)
{

    unsigned long vaddress = (unsigned long)address;
    struct mmap_header *header = (struct mmap_header *)KERNEL_VMMAP;
    struct mmap_entry *entry = mmap_find(header, vaddress);

    return (entry && (entry->flags & MMAP_FLAG_USERMODE) && (vaddress >= entry->vaddress) && (count <= entry->size) && ((vaddress - entry->vaddress) <= (entry->size - count)));

}

static unsigned int debug(unsigned int itask, void *stack)
{

    struct {void *caller; char *message;} *args = stack;

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "%s", args->message);

    return MESSAGE_OK;

}

static unsigned int kill(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int target;} *args = stack;
    struct mailbox *mailbox = mailbox_get(args->target);

    return (mailbox && mailbox->itask && kernel_kill(mailbox->itask)) ? MESSAGE_OK : MESSAGE_FAILED;

}

static unsigned int load(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int target; unsigned int id;} *args = stack;
    unsigned long address = (locate) ? locate(args->target, args->id) : 0;

    if (address)
    {

        struct binary_format *format = binary_findformat(address);

        if (format)
        {

            void (*module_init)(void) = (void (*)(void))(format->findsymbol(address, 11, "module_init"));
            void (*module_register)(void) = (void (*)(void))(format->findsymbol(address, 15, "module_register"));

            if (module_init)
                module_init();

            if (module_register)
                module_register();

            return MESSAGE_OK;

        }

    }

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "load failed");

    return MESSAGE_FAILED;

}

static unsigned int unload(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int target; unsigned int id;} *args = stack;
    unsigned long address = (locate) ? locate(args->target, args->id) : 0;

    if (address)
    {

        struct binary_format *format = binary_findformat(address);

        if (format)
        {

            void (*module_unregister)(void) = (void (*)(void))(format->findsymbol(address, 17, "module_unregister"));

            if (module_unregister)
                module_unregister();

            return MESSAGE_OK;

        }

    }

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "unload failed");

    return MESSAGE_FAILED;

}

static unsigned int spawn(unsigned int itask, void *stack)
{

    return 0;

}

static unsigned int despawn(unsigned int itask, void *stack)
{

    kernel_signal(itask, TASK_SIGNAL_KILL);

    return MESSAGE_OK;

}

static unsigned int pick(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int ichannel; struct message *messages; unsigned int count; unsigned int release; unsigned int flags;} *args = stack;

    if (args->count <= MESSAGE_SLOTS && (!args->count || (args->messages && checkargs(args->messages, sizeof (struct message) * args->count))))
    {

        unsigned int inode = kernel_getchannelinode(itask, args->ichannel);

        return (inode) ? mailbox_pick(inode, args->messages, args->count, args->release, args->flags) : MESSAGE_FAILED;

    }

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "pick check failed");

    return MESSAGE_FAILED;

}

static unsigned int place(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int ichannel; unsigned int target; unsigned int event; unsigned int count; void *data;} *args = stack;

    if ((args->data && args->count && checkargs(args->data, args->count)) || (!args->data && !args->count))
    {

        unsigned int inode = kernel_getchannelinode(itask, args->ichannel);
        unsigned int status = (inode) ? kernel_place(inode, args->target, args->event, args->count, args->data) : MESSAGE_FAILED;

        /* a full mailbox: the caller's library takes in its own mail, then sleeps in pick until a slot frees or mail arrives */
        if (status == MESSAGE_RETRY && mailbox_wait(args->target, itask))
            return MESSAGE_FULL;

        /* a kernel service could not answer into the caller's own full mailbox: the library takes in its own mail and tries again */
        if (status == MESSAGE_RETRY && mailbox_isfull(inode))
            return MESSAGE_FULL;

        return status;

    }

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "place check failed");

    return MESSAGE_FAILED;

}

static unsigned int announce(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int ichannel; unsigned int length; char *name;} *args = stack;

    if (args->name && args->length < SERVICE_NAMESIZE && checkargs(args->name, args->length))
    {

        unsigned int inode = kernel_getchannelinode(itask, args->ichannel);
        char name[SERVICE_NAMESIZE];

        buffer_clear(name, SERVICE_NAMESIZE);
        buffer_copy(name, args->name, args->length);

        return (inode) ? kernel_announce(inode, name) : MESSAGE_FAILED;

    }

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "announce check failed");

    return MESSAGE_FAILED;

}

unsigned int abi_call(unsigned int index, unsigned int itask, void *stack)
{

    return index < CALLS ? calls[index](itask, stack) : MESSAGE_FAILED;

}

void abi_setcallback(unsigned int index, unsigned int (*callback)(unsigned int itask, void *stack))
{

    calls[index] = callback;

}

void abi_setlocate(unsigned long (*callback)(unsigned int target, unsigned int id))
{

    locate = callback;

}

void abi_setup(void)
{

    abi_setcallback(0x00, debug);
    abi_setcallback(0x01, pick);
    abi_setcallback(0x02, place);
    abi_setcallback(0x03, spawn);
    abi_setcallback(0x04, despawn);
    abi_setcallback(0x05, kill);
    abi_setcallback(0x06, debug);
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

