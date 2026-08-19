#include <fudge.h>
#include "resource.h"
#include "debug.h"
#include "mmap.h"
#include "binary.h"
#include "mailbox.h"
#include "task.h"
#include "node.h"
#include "core.h"
#include "pool.h"
#include "kernel.h"

static struct list blockedtasks;
static struct core *(*getcorecallback)(void);
static void (*assigncorecallback)(unsigned int itask);

static struct core *getcore0(void)
{

    return pool_getcore(0);

}

static void assign0(unsigned int itask)
{

    struct core *core = pool_getcore(0);

    if (core)
        pool_placetask(itask, &core->tasks);

}

static void destroytask(unsigned int itask)
{

    struct task *task = pool_gettask(itask);

    if (task)
    {

        unsigned int i;

        for (i = 0; i < TASK_MAILBOXES; i++)
        {

            if (task->imailbox[i])
                pool_unpickmailbox(task->imailbox[i]);

        }

        task_resetmailboxes(task);
        pool_unpicktask(itask);

    }

}

static void assertstate(struct task *task, unsigned int state)
{

    if (task->state != state)
    {

        static char *names[7] = { "?", "DEAD", "NEW", "BLOCKED", "UNBLOCKED", "ASSIGNED", "RUNNING" };
        char *cname = names[task->state];
        char *ename = names[state];

        DEBUG_FMT0(DEBUG_ERROR, "Task state assertion failed");
        DEBUG_FMT1(DEBUG_NONE, "Current state: %s", cname);
        DEBUG_FMT1(DEBUG_NONE, "Expected state: %s", ename);

        for (;;);

    }

}

static void transition(unsigned int itask, unsigned int state)
{

    struct task *task = pool_gettask(itask);

    if (task)
    {

        spinlock_acquire(&task->spinlock);

        if (task_transition(task, state))
        {

            switch (task->state)
            {

            case TASK_STATE_DEAD:
                destroytask(itask);

                break;

            case TASK_STATE_ASSIGNED:
                assigncorecallback(itask);

                break;

            }

        }

        spinlock_release(&task->spinlock);

    }

}

static void unblocktasks(void)
{

    struct list_item *current;
    struct list_item *next;

    spinlock_acquire(&blockedtasks.spinlock);

    for (current = blockedtasks.head; current; current = next)
    {

        unsigned int itask = pool_getitaskfromitem(current);

        next = current->next;

        if (itask)
        {

            struct task *task = pool_gettask(itask);

            if (task)
            {

                assertstate(task, TASK_STATE_BLOCKED);

                if (task->signals.kills)
                {

                    list_remove_unsafe(&blockedtasks, current);
                    transition(itask, TASK_STATE_UNBLOCKED);
                    assertstate(task, TASK_STATE_UNBLOCKED);
                    transition(itask, TASK_STATE_DEAD);
                    assertstate(task, TASK_STATE_DEAD);

                }

                if (task->signals.unblocks)
                {

                    list_remove_unsafe(&blockedtasks, current);
                    transition(itask, TASK_STATE_UNBLOCKED);
                    assertstate(task, TASK_STATE_UNBLOCKED);
                    transition(itask, TASK_STATE_ASSIGNED);
                    assertstate(task, TASK_STATE_ASSIGNED);

                }

            }

        }

    }

    spinlock_release(&blockedtasks.spinlock);

}

struct core *kernel_getcore(void)
{

    return getcorecallback();

}

unsigned int kernel_getchannelinode(unsigned int itask, unsigned int ichannel)
{

    struct task *task = pool_gettask(itask);

    if (task && ichannel < TASK_MAILBOXES)
    {

        struct mailbox *mailbox = pool_getmailbox(task->imailbox[ichannel]);

        if (!mailbox)
        {

            task->imailbox[ichannel] = pool_pickmailbox();

            if (task->imailbox[ichannel])
            {

                mailbox = pool_getmailbox(task->imailbox[ichannel]);

                mailbox_reset(mailbox, itask);

            }

        }

        return (mailbox) ? mailbox->inode : 0;

    }

    return 0;

}

unsigned int kernel_linknode(unsigned int target, unsigned int source)
{

    struct node *snode = pool_getnode(source);
    struct node *tnode = pool_getnode(target);

    if (snode && tnode)
    {

        unsigned int inode = pool_picknode();

        if (inode)
        {

            struct node *node = pool_getnode(inode);

            node_reset(node, 0, snode->resource, snode->operands);
            pool_placenode(inode, &tnode->links);

            return MESSAGE_OK;

        }

    }

    return MESSAGE_FAILED;

}

unsigned int kernel_unlinknode(unsigned int target, unsigned int source)
{

    struct node *snode = pool_getnode(source);
    struct node *tnode = pool_getnode(target);

    if (snode && tnode)
    {

        struct list_item *current;
        struct list_item *next;

        spinlock_acquire(&tnode->links.spinlock);

        for (current = tnode->links.head; current; current = next)
        {

            unsigned int inode = pool_getinodefromitem(current);
            struct node *node = pool_getnode(inode);

            next = current->next;

            if (node->resource == snode->resource)
            {

                list_remove_unsafe(&tnode->links, current);
                pool_unpicknode(inode);

            }

        }

        spinlock_release(&tnode->links.spinlock);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

void kernel_schedule(struct core *core)
{

    if (core->itask)
    {

        struct task *task = pool_gettask(core->itask);

        if (task)
        {

            assertstate(task, TASK_STATE_RUNNING);

            if (task->signals.kills)
            {

                transition(core->itask, TASK_STATE_DEAD);
                assertstate(task, TASK_STATE_DEAD);

                core->itask = 0;

            }

            if (task->signals.blocks)
            {

                transition(core->itask, TASK_STATE_BLOCKED);
                assertstate(task, TASK_STATE_BLOCKED);
                pool_placetask(core->itask, &blockedtasks);

                core->itask = 0;

            }

            if (core->itask)
            {

                assertstate(task, TASK_STATE_RUNNING);
                transition(core->itask, TASK_STATE_ASSIGNED);
                assertstate(task, TASK_STATE_ASSIGNED);

                core->itask = 0;

            }

        }

    }

    unblocktasks();

    core->itask = pool_picktaskfrom(&core->tasks);

    if (core->itask)
    {

        struct task *task = pool_gettask(core->itask);

        if (task)
        {

            assertstate(task, TASK_STATE_ASSIGNED);
            transition(core->itask, TASK_STATE_RUNNING);
            assertstate(task, TASK_STATE_RUNNING);

        }

    }

}

void kernel_signal(unsigned int itask, unsigned int signal)
{

    struct task *task = pool_gettask(itask);

    if (task)
    {

        spinlock_acquire(&task->spinlock);
        task_signal(task, signal);
        spinlock_release(&task->spinlock);

    }

}

unsigned int kernel_pick(unsigned int source, struct message *message, unsigned int count, void *data)
{

    struct node *snode = pool_getnode(source);

    return (snode && snode->operands && snode->operands->pick) ? snode->operands->pick(snode->resource, source, message, count, data) : MESSAGE_FAILED;

}

unsigned int kernel_place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct node *tnode = pool_getnode(target);

    return (tnode && tnode->operands && tnode->operands->place) ? tnode->operands->place(tnode->resource, source, target, event, count, data) : MESSAGE_FAILED;

}

unsigned int kernel_announce(unsigned int inode, unsigned int namehash)
{

    struct node *node = pool_getnode(inode);

    return (node) ? node->namehash = namehash : 0;

}

void kernel_notify(unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct node *snode = pool_getnode(source);

    if (snode)
    {

        struct list *links = &snode->links;
        struct list_item *current = 0;

        spinlock_acquire(&links->spinlock);

        while ((current = list_next_unsafe(links, current)))
        {

            unsigned int target = pool_getinodefromitem(current);

            if (target)
                kernel_place(source, target, event, count, data);

        }

        spinlock_release(&links->spinlock);

    }

}

unsigned int kernel_loadtask(unsigned int itask, unsigned int ip, unsigned int sp, unsigned int address)
{

    struct task *task = pool_gettask(itask);
    unsigned int inode = 0;

    if (task)
    {

        task_reset(task);

        task->thread.ip = ip;
        task->thread.sp = sp;
        task->address = address;

        if (task->address)
        {

            struct binary_format *format = binary_findformat(task->address);

            if (format)
                task->thread.ip = format->findentry(task->address);

        }

        if (task->thread.ip)
        {

            inode = kernel_getchannelinode(itask, 0);

            if (inode)
            {

                transition(itask, TASK_STATE_NEW);
                assertstate(task, TASK_STATE_NEW);
                transition(itask, TASK_STATE_ASSIGNED);
                assertstate(task, TASK_STATE_ASSIGNED);

            }

        }

    }

    return inode;

}

void kernel_setcallback(struct core *(*getcore)(void), void (*assigncore)(unsigned int itask))
{

    getcorecallback = getcore;
    assigncorecallback = assigncore;

}

void kernel_setup(void)
{

    list_init(&blockedtasks);
    kernel_setcallback(getcore0, assign0);

}

