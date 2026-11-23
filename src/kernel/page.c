#include <fudge.h>
#include "resource.h"
#include "task.h"
#include "mmap.h"
#include "pool.h"
#include "kernel.h"
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
static struct pagequeue queues[POOL_TASKS];
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

unsigned int page_give(unsigned int itask, unsigned int page, unsigned int target, unsigned int count, unsigned int source)
{

    struct task *task = pool_gettask(target);
    unsigned int given = 0;

    if (!isvalid(page) || !task || count > PAGE_SIZE)
        return 0;

    spinlock_acquire(&spinlock);

    /* a task marked dead may not have handed back its pages yet, but must get no more */
    if (rows[page].owner == itask && !rows[page].queued && task->state != TASK_STATE_DEAD)
    {

        rows[page].owner = target;
        rows[page].count = count;
        rows[page].source = source;
        rows[page].queued = 1;
        rows[page].next = 0;

        if (queues[target].tail)
            rows[queues[target].tail].next = page;
        else
            queues[target].head = page;

        queues[target].tail = page;
        given = 1;

    }

    spinlock_release(&spinlock);

    if (given)
        kernel_signal(target, TASK_SIGNAL_UNBLOCK);

    return given;

}

struct page_handoff page_take(unsigned int itask)
{

    struct page_handoff handoff;

    handoff.page = 0;
    handoff.count = 0;
    handoff.source = 0;

    spinlock_acquire(&spinlock);

    if (queues[itask].head)
    {

        unsigned int page = queues[itask].head;

        queues[itask].head = rows[page].next;

        if (!queues[itask].head)
            queues[itask].tail = 0;

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
    queues[itask].head = 0;
    queues[itask].tail = 0;
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

}

