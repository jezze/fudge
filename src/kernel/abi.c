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
#include "page.h"

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

static unsigned int place(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int ichannel; unsigned int target; unsigned int event; unsigned int count; void *data;} *args = stack;

    if ((args->data && args->count && checkargs(args->data, args->count)) || (!args->data && !args->count))
    {

        unsigned int inode = kernel_getchannelinode(itask, args->ichannel);
        struct mailbox *mailbox = pool_getnodemailbox(inode);
        struct mailbox *target = pool_getnodemailbox(args->target);
        unsigned int status;

        if (!mailbox)
            return MESSAGE_FAILED;

        if (target)
        {

            struct task *task = pool_gettask(itask);

            /* a full target never puts the sender to sleep, it retries; the target is woken in case it waits on another channel and must run its full channel 0 */
            status = kernel_place(inode, args->target, args->event, args->count, args->data);

            if (status == MESSAGE_RETRY)
            {

                kernel_signal(target->itask, TASK_SIGNAL_UNBLOCK);

                if (++task->retries == TASK_RETRIESLOG)
                    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "task %u keeps retrying a send to task %u", &itask, &target->itask);

            }

            else
                task->retries = 0;

            return status;

        }

        /* a kernel server answers into a slot of the caller's mailbox reserved before any work is done; no room even after reclaiming: the caller sleeps until mail arrives there or a sender to it is refused, then places again */
        status = mailbox_reservereply(mailbox);

        if (status == MESSAGE_RETRY)
            mailbox_sleep(mailbox);

        if (status != MESSAGE_OK)
            return status;

        status = kernel_place(inode, args->target, args->event, args->count, args->data);

        mailbox_unreservereply(mailbox);

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

static unsigned int pagealloc(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int flags;} *args = stack;
    unsigned int page = page_alloc(itask);

    if (!page && (args->flags & PAGE_FLAG_BLOCK))
        kernel_signal(itask, TASK_SIGNAL_BLOCK);

    return page;

}

static unsigned int pagegive(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int page; unsigned int target; unsigned int count;} *args = stack;

    /* the receiver learns who gave the page by the giver's channel 0, as a message names its sender */
    return page_give(itask, args->page, args->target, args->count, kernel_getchannelinode(itask, 0));

}

static unsigned int pagetake(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int ichannel; struct page_handoff *handoff; unsigned int flags;} *args = stack;

    if (args->handoff && checkargs(args->handoff, sizeof (struct page_handoff)))
    {

        struct page_handoff handoff = page_take(itask, args->ichannel);

        if (handoff.page)
        {

            buffer_copy(args->handoff, &handoff, sizeof (struct page_handoff));

            return MESSAGE_OK;

        }

        if (args->flags & PAGE_FLAG_BLOCK)
            kernel_signal(itask, TASK_SIGNAL_BLOCK);

        return MESSAGE_RETRY;

    }

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "pagetake check failed");

    return MESSAGE_FAILED;

}

static unsigned int pagehand(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int page; unsigned int target;} *args = stack;

    return page_hand(itask, args->page, args->target);

}

static unsigned int pagefree(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int page;} *args = stack;

    return (page_free(itask, args->page)) ? MESSAGE_OK : MESSAGE_FAILED;

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
    abi_setcallback(0x0A, pagealloc);
    abi_setcallback(0x0B, pagegive);
    abi_setcallback(0x0C, pagetake);
    abi_setcallback(0x0D, pagefree);
    abi_setcallback(0x0E, pagehand);
    abi_setcallback(0x0F, debug);

}

