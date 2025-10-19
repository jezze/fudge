#include <fudge.h>
#include "resource.h"
#include "mmap.h"
#include "binary.h"
#include "mailbox.h"
#include "task.h"
#include "node.h"
#include "core.h"
#include "pool.h"
#include "kernel.h"

static struct list blockedtasks;
static struct core *(*getcallback)(void);
static void (*assigncallback)(struct list_item *item);
static struct core core0;

static void assign(struct list_item *item)
{

    if (assigncallback)
        assigncallback(item);
    else
        list_add(&core0.tasks, item);

}

static void checkstate(unsigned int itask)
{

    struct task *task = pool_gettask(itask);

    if (task)
    {

        struct list_item *item = pool_gettaskitem(itask);

        switch (task->state)
        {

        case TASK_STATE_DEAD:
            pool_destroytask(itask);

            break;

        case TASK_STATE_BLOCKED:
            list_add(&blockedtasks, item);

            break;

        case TASK_STATE_UNBLOCKED:
            task_transition(task, TASK_STATE_ASSIGNED);
            assign(item);

            break;

        case TASK_STATE_NEW:
            task_transition(task, TASK_STATE_ASSIGNED);
            assign(item);

            break;

        case TASK_STATE_RUNNING:
            task_transition(task, TASK_STATE_ASSIGNED);
            assign(item);

            break;

        }

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
        struct task *task = pool_gettask(itask);

        next = current->next;

        task_checksignals(task);

        if (task->state == TASK_STATE_UNBLOCKED)
        {

            list_remove_unsafe(&blockedtasks, current);
            checkstate(itask);

        }

    }

    spinlock_release(&blockedtasks.spinlock);

}

static unsigned int picknewtask(struct core *core)
{

    struct list_item *item = list_pickhead(&core->tasks);

    if (item)
    {

        unsigned int itask = pool_getitaskfromitem(item);
        struct task *task = pool_gettask(itask);

        task_transition(task, TASK_STATE_RUNNING);

        return itask;

    }

    return 0;

}

struct task_thread *kernel_gettaskthread(unsigned int itask)
{

    struct task *task = pool_gettask(itask);

    return (task) ? &task->thread : 0;

}

struct core *kernel_getcore(void)
{

    return (getcallback) ? getcallback() : &core0;

}

unsigned int kernel_getchannelinode(unsigned int itask, unsigned int ichannel)
{

    struct task *task = pool_gettask(itask);

    if (task)
    {

        struct mailbox *mailbox = pool_getmailbox(task->imailbox[ichannel]);

        if (!mailbox)
        {

            task->imailbox[ichannel] = pool_addmailbox(itask);
            mailbox = pool_getmailbox(task->imailbox[ichannel]);

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

        unsigned int inode = pool_addnodeto(&tnode->links, "link", snode->resource, snode->operands);

        if (inode)
            return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

unsigned int kernel_unlinknode(unsigned int target, unsigned int source)
{

    struct node *snode = pool_getnode(source);
    struct node *tnode = pool_getnode(target);

    if (snode && tnode)
    {

        pool_removenodefrom(&tnode->links, source);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

unsigned int kernel_schedule(struct core *core)
{

    struct task *task = pool_gettask(core->itask);

    if (task)
    {

        task_checksignals(task);
        checkstate(core->itask);

    }

    unblocktasks();

    return picknewtask(core);

}

void kernel_signal(unsigned int itask, unsigned int signal)
{

    struct task *task = pool_gettask(itask);

    if (task)
        task_signal(task, signal);

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
        struct list_item *current;

        spinlock_acquire(&links->spinlock);

        for (current = links->head; current; current = current->next)
        {

            unsigned int inode = pool_getinodefromitem(current);

            kernel_place(source, inode, event, count, data);

        }

        spinlock_release(&links->spinlock);

    }

}

unsigned int kernel_loadtask(unsigned int itask, unsigned int ip, unsigned int sp, unsigned int address)
{

    struct task *task = pool_gettask(itask);

    if (task)
    {

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
            task_transition(task, TASK_STATE_NEW);

        checkstate(itask);

        if (task->thread.ip)
        {

            task->imailbox[0] = pool_addmailbox(itask);

            if (task->imailbox[0])
            {

                struct mailbox *mailbox = pool_getmailbox(task->imailbox[0]);

                return mailbox->inode;

            }

        }

    }

    return 0;

}

void kernel_setcallback(struct core *(*get)(void), void (*assign)(struct list_item *item))
{

    getcallback = get;
    assigncallback = assign;

}

void kernel_setup(unsigned int saddress, unsigned int ssize)
{

    list_init(&blockedtasks);
    core_init(&core0, 0, saddress + ssize);

}

