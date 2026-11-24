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

    unsigned int free;
    unsigned int origin;
    unsigned int owner;
    unsigned int count;
    unsigned int source;
    unsigned int queued;
    unsigned int next;
    struct message message;
    struct list_item item;

};

/* pages are numbered from 1, so row 0 means none; a given page waits in the receiver's mailbox as its own message, in order with the mail */
static struct pagerow rows[PAGE_COUNT + 1];
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

    if (rows[page].free)
        debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "Page released twice");

    if (rows[page].origin)
        charged[rows[page].origin]--;

    rows[page].origin = 0;
    rows[page].owner = 0;
    rows[page].count = 0;
    rows[page].source = 0;
    rows[page].queued = 0;
    rows[page].next = freehead;
    rows[page].free = 1;
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

unsigned int page_getheld(unsigned int itask)
{

    unsigned int count = 0;
    unsigned int page;

    for (page = 1; page <= PAGE_COUNT; page++)
    {

        if (rows[page].owner == itask)
            count++;

    }

    return count;

}

struct page_info page_getinfo(unsigned int page)
{

    struct page_info info;

    info.owner = (isvalid(page)) ? rows[page].owner : 0;
    info.origin = (isvalid(page)) ? rows[page].origin : 0;
    info.queued = (isvalid(page)) ? rows[page].queued : 0;
    info.count = (isvalid(page)) ? rows[page].count : 0;
    info.source = (isvalid(page)) ? rows[page].source : 0;

    return info;

}

unsigned int page_alloc(unsigned int itask)
{

    unsigned int page;

    spinlock_acquire(&spinlock);

    page = (charged[itask] < PAGE_TASKLIMIT) ? freehead : 0;

    if (page)
    {

        if (!rows[page].free || rows[page].owner)
            debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "Page taken from the pool is in use");

        rows[page].free = 0;
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

/* the page goes to the task as a message of its own, in order with the mail, from the channel it was sent on */
unsigned int page_send(unsigned int itask, unsigned int page, struct mailbox *mailbox, unsigned int event, unsigned int count, unsigned int source)
{

    struct task *task = pool_gettask(mailbox->itask);
    unsigned int status = MESSAGE_FAILED;

    if (!task || !isvalid(page) || count > PAGE_SIZE)
        return MESSAGE_FAILED;

    spinlock_acquire(&spinlock);

    /* a task marked dead may not have handed back its pages yet, but must get no more */
    if (rows[page].owner == itask && !rows[page].queued && task->state != TASK_STATE_DEAD)
    {

        rows[page].owner = mailbox->itask;
        rows[page].count = count;
        rows[page].source = source;
        rows[page].queued = 1;
        status = MESSAGE_OK;

    }

    spinlock_release(&spinlock);

    if (status != MESSAGE_OK)
        return status;

    message_init(&rows[page].message, event, source, count, (void *)(PAGE_VBASE + (unsigned long)(page - 1) * PAGE_SIZE));

    rows[page].message.slot = MESSAGE_SLOTS;

    list_inititem(&rows[page].item, &rows[page].message);
    spinlock_acquire(&mailbox->spinlock);
    list_add_unsafe(&mailbox->queued, &rows[page].item);
    spinlock_release(&mailbox->spinlock);
    kernel_signal(mailbox->itask, TASK_SIGNAL_UNBLOCK);

    return MESSAGE_OK;

}

/* the receiver picked the page's message from its mailbox: from now it holds the page */
void page_picked(void *data)
{

    unsigned long address = (unsigned long)data;
    unsigned int page = (address >= PAGE_VBASE) ? (address - PAGE_VBASE) / PAGE_SIZE + 1 : 0;

    if (!isvalid(page))
        return;

    spinlock_acquire(&spinlock);

    rows[page].queued = 0;

    spinlock_release(&spinlock);

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

/* a reader brings its own page with a request: handed to a task, that task now holds it; a service in the kernel uses it where it is */
unsigned int page_hand(unsigned int itask, unsigned int page, unsigned int target)
{

    struct mailbox *mailbox = pool_getnodemailbox(target);
    struct task *task = (mailbox) ? pool_gettask(mailbox->itask) : 0;
    unsigned int status = MESSAGE_FAILED;

    if (!isvalid(page))
        return MESSAGE_FAILED;

    spinlock_acquire(&spinlock);

    /* as with give: a task marked dead may already have handed back its pages, and a page made its own now would never come back */
    if (rows[page].owner == itask && !rows[page].queued && (!task || task->state != TASK_STATE_DEAD))
    {

        if (mailbox)
            rows[page].owner = mailbox->itask;

        status = MESSAGE_OK;

    }

    spinlock_release(&spinlock);

    return status;

}

struct page_lease page_borrow(unsigned int source, unsigned int page)
{

    struct mailbox *mailbox = pool_getnodemailbox(source);
    struct page_lease lease;

    lease.page = 0;
    lease.status = MESSAGE_FAILED;
    lease.data = 0;

    if (!mailbox || !isvalid(page))
        return lease;

    spinlock_acquire(&spinlock);

    if (rows[page].owner == mailbox->itask && !rows[page].queued)
    {

        lease.page = page;
        lease.status = MESSAGE_OK;
        lease.data = (void *)(base + (unsigned long)(page - 1) * PAGE_SIZE);

    }

    spinlock_release(&spinlock);

    return lease;

}

unsigned int page_answer(unsigned int target, struct page_lease *lease, unsigned int count, unsigned int source)
{

    struct mailbox *mailbox = pool_getnodemailbox(target);

    if (!mailbox)
        return MESSAGE_FAILED;

    return page_send(mailbox->itask, lease->page, mailbox, MESSAGE_TYPE_DATA, count, source);

}

/* the task's mailboxes were reset already, so pages still waiting in them are released with the rest */
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

