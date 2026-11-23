#include <fudge.h>
#include "resource.h"
#include "task.h"
#include "mmap.h"
#include "pool.h"
#include "service.h"
#include "mailbox.h"
#include "kernel.h"
#include "debug.h"
#include "page.h"

struct pagerow
{

    unsigned int origin;
    unsigned int owner;
    unsigned int count;
    unsigned int source;
    unsigned int queued;
    unsigned int next;

};

struct pagequeue
{

    unsigned int head;
    unsigned int tail;

};

/* pages are numbered from 1, so row 0 and an empty queue both mean none */
static struct pagerow rows[PAGE_COUNT + 1];
/* pages are given to a channel, like mail, so a task takes them where it waits */
static struct pagequeue queues[POOL_TASKS][TASK_MAILBOXES];
static unsigned char waiting[POOL_TASKS];
/* a page counts against the task that took it from the pool until someone frees it, wherever it is by then */
static unsigned int charged[POOL_TASKS];
static unsigned long base;
static unsigned int freehead;
static unsigned int nfree;
static struct spinlock spinlock;

static unsigned int isvalid(unsigned int page)
{

    return page && page <= PAGE_COUNT;

}

static void release(unsigned int page)
{

    if (rows[page].origin)
        charged[rows[page].origin]--;

    rows[page].origin = 0;
    rows[page].owner = 0;
    rows[page].count = 0;
    rows[page].source = 0;
    rows[page].queued = 0;
    rows[page].next = freehead;
    freehead = page;
    nfree++;

}

static void wakewaiting(void)
{

    unsigned int itask;

    for (itask = 1; itask < POOL_TASKS; itask++)
    {

        if (waiting[itask])
        {

            waiting[itask] = 0;

            kernel_signal(itask, TASK_SIGNAL_UNBLOCK);

        }

    }

}

unsigned long page_getbase(void)
{

    return base;

}

unsigned int page_getfree(void)
{

    return nfree;

}

unsigned int page_alloc(unsigned int itask)
{

    unsigned int page;

    spinlock_acquire(&spinlock);

    page = (charged[itask] < PAGE_TASKLIMIT) ? freehead : 0;

    if (page)
    {

        freehead = rows[page].next;
        nfree--;
        charged[itask]++;
        rows[page].origin = itask;
        rows[page].owner = itask;
        rows[page].next = 0;

    }

    /* marked while still locked, so a page freed right after this wakes the task */
    else
        waiting[itask] = 1;

    spinlock_release(&spinlock);

    return page;

}

static unsigned int givetask(unsigned int itask, unsigned int page, unsigned int target, unsigned int ichannel, unsigned int count, unsigned int source)
{

    struct task *task = pool_gettask(target);
    unsigned int status = MESSAGE_FAILED;

    if (!task || ichannel >= TASK_MAILBOXES)
        return MESSAGE_FAILED;

    spinlock_acquire(&spinlock);

    /* a task marked dead may not have handed back its pages yet, but must get no more */
    if (rows[page].owner == itask && !rows[page].queued && task->state != TASK_STATE_DEAD)
    {

        struct pagequeue *queue = &queues[target][ichannel];

        rows[page].owner = target;
        rows[page].count = count;
        rows[page].source = source;
        rows[page].queued = 1;
        rows[page].next = 0;

        if (queue->tail)
            rows[queue->tail].next = page;
        else
            queue->head = page;

        queue->tail = page;
        status = MESSAGE_OK;

    }

    spinlock_release(&spinlock);

    if (status == MESSAGE_OK)
        kernel_signal(target, TASK_SIGNAL_UNBLOCK);

    return status;

}

/* a service in the kernel reads the page in place while it is given; it is freed once taken, or stays with the giver on a retry */
static unsigned int giveservice(unsigned int itask, unsigned int page, unsigned int target, unsigned int count, unsigned int source)
{

    unsigned int owned;
    unsigned int status;

    spinlock_acquire(&spinlock);

    owned = rows[page].owner == itask && !rows[page].queued;

    spinlock_release(&spinlock);

    if (!owned)
        return MESSAGE_FAILED;

    status = kernel_place(source, target, MESSAGE_TYPE_DATA, count, (void *)(base + (unsigned long)(page - 1) * PAGE_SIZE));

    if (status == MESSAGE_OK)
        page_free(itask, page);

    return status;

}

unsigned int page_give(unsigned int itask, unsigned int page, unsigned int target, unsigned int count, unsigned int source)
{

    struct mailbox *mailbox = pool_getnodemailbox(target);

    if (!isvalid(page) || count > PAGE_SIZE)
        return MESSAGE_FAILED;

    return (mailbox) ? givetask(itask, page, mailbox->itask, mailbox->ichannel, count, source) : giveservice(itask, page, target, count, source);

}

struct page_handoff page_take(unsigned int itask, unsigned int ichannel)
{

    struct page_handoff handoff;

    handoff.page = 0;
    handoff.count = 0;
    handoff.source = 0;

    if (ichannel >= TASK_MAILBOXES)
        return handoff;

    spinlock_acquire(&spinlock);

    if (queues[itask][ichannel].head)
    {

        struct pagequeue *queue = &queues[itask][ichannel];
        unsigned int page = queue->head;

        queue->head = rows[page].next;

        if (!queue->head)
            queue->tail = 0;

        rows[page].queued = 0;
        rows[page].next = 0;
        handoff.page = page;
        handoff.count = rows[page].count;
        handoff.source = rows[page].source;

    }

    spinlock_release(&spinlock);

    return handoff;

}

unsigned int page_free(unsigned int itask, unsigned int page)
{

    unsigned int freed = 0;

    if (!isvalid(page))
        return 0;

    spinlock_acquire(&spinlock);

    if (rows[page].owner == itask && !rows[page].queued)
    {

        release(page);
        wakewaiting();

        freed = 1;

    }

    spinlock_release(&spinlock);

    return freed;

}

void page_untask(unsigned int itask)
{

    unsigned int ichannel;
    unsigned int page;

    spinlock_acquire(&spinlock);

    for (page = 1; page <= PAGE_COUNT; page++)
    {

        if (rows[page].owner == itask)
            release(page);
        else if (rows[page].origin == itask)
            rows[page].origin = 0;

    }

    charged[itask] = 0;

    for (ichannel = 0; ichannel < TASK_MAILBOXES; ichannel++)
    {

        queues[itask][ichannel].head = 0;
        queues[itask][ichannel].tail = 0;

    }

    waiting[itask] = 0;

    wakewaiting();
    spinlock_release(&spinlock);

}

void page_setup(unsigned long address)
{

    unsigned int page;

    spinlock_init(&spinlock);

    base = address;
    freehead = 0;
    nfree = 0;

    for (page = PAGE_COUNT; page > 0; page--)
        release(page);

    /* every task must be able to reach its limit at once, or tasks waiting for pages could wait on each other */
    if (PAGE_TASKLIMIT * (POOL_TASKS - 1) > PAGE_COUNT)
        debug_fmt(DEBUG_CRITICAL, __FILE__, __LINE__, "The page pool is smaller than every task at its limit");

}

