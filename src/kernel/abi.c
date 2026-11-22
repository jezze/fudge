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

    return (kernel_kill(kernel_getinodetask(args->target))) ? MESSAGE_OK : MESSAGE_FAILED;

}

static void callmodule(struct binary_format *format, unsigned long address, char *name)
{

    void (*call)(void) = (void (*)(void))(format->findsymbol(address, cstring_length(name), name));

    if (call)
        call();

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

            callmodule(format, address, "module_init");
            callmodule(format, address, "module_register");

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

            callmodule(format, address, "module_unregister");

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

static unsigned int raise(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int target; unsigned int bits;} *args = stack;
    struct mailbox *mailbox = pool_getnodemailbox(args->target);

    if (!mailbox || !mailbox->itask)
        return MESSAGE_FAILED;

    kernel_raise(mailbox->itask, args->bits);

    return MESSAGE_OK;

}

static unsigned int collect(unsigned int itask, void *stack)
{

    return kernel_collect(itask);

}

static unsigned int despawn(unsigned int itask, void *stack)
{

    kernel_signal(itask, TASK_SIGNAL_KILL);

    return MESSAGE_OK;

}

static unsigned int pick(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int ichannel; struct message *messages; unsigned int count; unsigned int flags;} *args = stack;

    if (args->count <= MESSAGE_SLOTS && (!args->count || (args->messages && checkargs(args->messages, sizeof (struct message) * args->count))))
    {

        struct mailbox *mailbox = pool_getnodemailbox(kernel_getchannelinode(itask, args->ichannel));

        return (mailbox) ? mailbox_pick(mailbox, args->messages, args->count, args->flags) : MESSAGE_FAILED;

    }

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "pick check failed");

    return MESSAGE_FAILED;

}

static unsigned int send(unsigned int itask, unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data, struct message *reply)
{

    unsigned int inode = kernel_getchannelinode(itask, ichannel);
    struct mailbox *mailbox = pool_getnodemailbox(inode);
    struct mailbox *receiver = pool_getnodemailbox(target);
    struct task *task = pool_gettask(itask);
    unsigned int status;
    unsigned int replied;

    if (!mailbox || !task)
        return MESSAGE_FAILED;

    if (receiver)
    {

        /* a full target never puts the sender to sleep: it retries, or posts and is told when there is room */
        status = kernel_place(inode, target, event, count, data);

        if (status == MESSAGE_RETRY)
        {

            mailbox_refuse(receiver, itask);

            if (++task->retries == TASK_RETRIESLOG)
                debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "task %u keeps retrying a send to task %u", &itask, &receiver->itask);

        }

        else
            task->retries = 0;

        /* a task answers a call with mail */
        return (reply && status == MESSAGE_OK) ? MESSAGE_PENDING : status;

    }

    /* a kernel server answers into a slot of the caller's mailbox reserved before any work is done; no room even after reclaiming: the caller sleeps until mail arrives there or a sender to it is refused, then places again */
    status = mailbox_reservereply(mailbox);

    if (status == MESSAGE_RETRY)
        mailbox_sleep(mailbox);

    if (status != MESSAGE_OK)
        return status;

    task->reply = reply;
    task->replychannel = ichannel;
    status = kernel_place(inode, target, event, count, data);
    replied = reply && !task->reply;
    task->reply = 0;

    mailbox_unreservereply(mailbox);

    /* a server that did not answer straight into the reply answers with mail, now or later */
    if (replied)
        return MESSAGE_OK;

    return (reply && status == MESSAGE_OK) ? MESSAGE_PENDING : status;

}

static unsigned int place(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int ichannel; unsigned int target; unsigned int event; unsigned int count; void *data;} *args = stack;

    if ((args->data && args->count && checkargs(args->data, args->count)) || (!args->data && !args->count))
        return send(itask, args->ichannel, args->target, args->event, args->count, args->data, 0);

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "place check failed");

    return MESSAGE_FAILED;

}

static unsigned int call(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int ichannel; unsigned int target; unsigned int event; unsigned int count; void *data; struct message *reply;} *args = stack;

    if (((args->data && args->count && checkargs(args->data, args->count)) || (!args->data && !args->count)) && args->reply && checkargs(args->reply, sizeof (struct message)) && args->reply->length && checkargs(args->reply->data, args->reply->length))
        return send(itask, args->ichannel, args->target, args->event, args->count, args->data, args->reply);

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "call check failed");

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
    abi_setcallback(0x06, raise);
    abi_setcallback(0x07, load);
    abi_setcallback(0x08, unload);
    abi_setcallback(0x09, announce);
    abi_setcallback(0x0A, collect);
    abi_setcallback(0x0B, call);
    abi_setcallback(0x0C, debug);
    abi_setcallback(0x0D, debug);
    abi_setcallback(0x0E, debug);
    abi_setcallback(0x0F, debug);

}

