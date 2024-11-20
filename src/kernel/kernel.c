#include <fudge.h>
#include "resource.h"
#include "debug.h"
#include "binary.h"
#include "mailbox.h"
#include "task.h"
#include "core.h"
#include "service.h"
#include "kernel.h"

static struct mailboxrow {struct list_item item; struct mailbox mailbox;} mailboxrows[KERNEL_MAILBOXES];
static struct taskrow {struct list_item item; struct task task;} taskrows[KERNEL_TASKS];
static struct noderow {struct list_item item; struct mailbox *mailbox; struct resource *resource; struct list targets; unsigned int (*place)(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data);} noderows[KERNEL_NODES];
static struct list freenodes;
static struct list deadtasks;
static struct list blockedtasks;
static struct list usednodes;
static struct list freemailboxes;
static struct list usedmailboxes;
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

static struct mailbox *getmailbox(unsigned int inode)
{

    return (inode && inode < KERNEL_NODES) ? noderows[inode].mailbox : 0;

}

static struct task *gettask(unsigned int itask)
{

    return (itask && itask < KERNEL_TASKS) ? &taskrows[itask].task : 0;

}

static unsigned int encodetaskrow(struct taskrow *taskrow)
{

    unsigned int i;

    for (i = 1; i < KERNEL_TASKS; i++)
    {

        if (&taskrows[i] == taskrow)
            return i;

    }

    return 0;

}

static unsigned int encodenoderow(struct noderow *noderow)
{

    unsigned int i;

    for (i = 1; i < KERNEL_NODES; i++)
    {

        if (&noderows[i] == noderow)
            return i;

    }

    return 0;

}

static void unblocktasks(void)
{

    struct list_item *taskrowitem;
    struct list_item *next;

    spinlock_acquire(&blockedtasks.spinlock);

    for (taskrowitem = blockedtasks.head; taskrowitem; taskrowitem = next)
    {

        struct taskrow *taskrow = taskrowitem->data;
        struct task *task = &taskrow->task;

        next = taskrowitem->next;

        if (task->signals.unblocks)
        {

            if (task_transition(task, TASK_STATE_UNBLOCKED))
            {

                list_remove_unsafe(&blockedtasks, taskrowitem);

                if (task_transition(task, TASK_STATE_ASSIGNED))
                    assign(taskrowitem);

            }

        }

    }

    spinlock_release(&blockedtasks.spinlock);

}

static void checksignals(struct core *core, struct taskrow *taskrow)
{

    struct task *task = &taskrow->task;
    struct list_item *item = &taskrow->item;

    if (task->signals.kills)
    {

        if (task_transition(task, TASK_STATE_DEAD))
            list_add(&deadtasks, item);

    }

    else if (task->signals.unblocks)
    {

        if (task_transition(task, TASK_STATE_ASSIGNED))
            list_add(&core->tasks, item);

    }

    else if (task->signals.blocks)
    {

        if (task_transition(task, TASK_STATE_BLOCKED))
            list_add(&blockedtasks, item);

    }

    else
    {

        if (task_transition(task, TASK_STATE_ASSIGNED))
            list_add(&core->tasks, item);

    }

    task_resetsignals(&task->signals);

}

static unsigned int placetask(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct task *task = kernel_getinterface(target);
    struct mailbox *mailbox = getmailbox(target);

    if (task && mailbox)
    {

        struct message message;
        unsigned int status;

        message_init(&message, event, source, count);

        status = mailbox_place(mailbox, &message, data);

        task_signal(task, TASK_SIGNAL_UNBLOCK);

        return status;

    }

    return MESSAGE_FAILED;

}

static void *pickrow(struct list *from, struct list *to)
{

    struct list_item *item = list_picktail(from);

    if (item)
    {

        list_add(to, item);

        return item->data;

    }

    return 0;

}

static void returnrow(struct list *from, struct list *to, struct list_item *item)
{

    struct list_item *current;
    struct list_item *next;

    spinlock_acquire(&from->spinlock);

    for (current = from->head; current; current = next)
    {

        next = current->next;

        if (current == item)
        {

            list_remove_unsafe(from, item);
            list_add(to, item);

        }

    }

    spinlock_release(&from->spinlock);

}

static struct mailbox *pickmailbox(void)
{

    struct mailboxrow *mailboxrow = pickrow(&freemailboxes, &usedmailboxes);

    if (mailboxrow)
    {

        struct mailbox *mailbox = &mailboxrow->mailbox;

        mailbox_reset(mailbox);

        return mailbox;

    }

    return 0;

}

static unsigned int picknewtask(struct core *core)
{

    struct list_item *taskrowitem = list_picktail(&core->tasks);

    if (taskrowitem)
    {

        struct taskrow *taskrow = taskrowitem->data;
        struct task *task = &taskrow->task;

        if (task_transition(task, TASK_STATE_RUNNING))
            return encodetaskrow(taskrow);

    }

    return 0;

}

void *kernel_getinterface(unsigned int inode)
{

    struct resource *resource = (inode && inode < KERNEL_NODES) ? noderows[inode].resource : 0;

    return (resource) ? resource->data : 0;

}

struct core *kernel_getcore(void)
{

    return (getcallback) ? getcallback() : &core0;

}

void kernel_setcallback(struct core *(*get)(void), void (*assign)(struct list_item *item))
{

    getcallback = get;
    assigncallback = assign;

}

unsigned int kernel_link(struct list *nodes, struct mailbox *mailbox, struct resource *resource, unsigned int (*place)(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data))
{

    struct noderow *noderow = pickrow(&freenodes, (nodes) ? nodes : &usednodes);

    if (noderow)
    {

        list_init(&noderow->targets);

        noderow->resource = resource;
        noderow->mailbox = mailbox;
        noderow->place = place;

        return encodenoderow(noderow);

    }

    return 0;

}

unsigned int kernel_unlink(struct list *nodes, unsigned int inode)
{

    returnrow((nodes) ? nodes : &usednodes, &freenodes, &noderows[inode].item);

    return 0;

}

unsigned int kernel_schedule(struct core *core)
{

    if (core->itask)
        checksignals(core, &taskrows[core->itask]);

    unblocktasks();

    return picknewtask(core);

}

unsigned int kernel_codebase(unsigned int itask, unsigned int address)
{

    struct task *task = gettask(itask);

    if (task)
    {

        struct binary_format *format = binary_findformat(task->base);

        return (format) ? format->findbase(task->base, address) : 0;

    }

    return 0;

}

unsigned int kernel_loadprogram(unsigned int itask)
{

    struct task *task = gettask(itask);

    if (task)
    {

        struct binary_format *format = binary_findformat(task->base);

        return (format) ? format->copyprogram(task->base) : 0;

    }

    return 0;

}

void kernel_signal(unsigned int itask, unsigned int signal)
{

    struct task *task = gettask(itask);

    if (task)
        task_signal(task, signal);

}

struct task_thread *kernel_getthread(unsigned int itask)
{

    struct task *task = gettask(itask);

    return (task) ? &task->thread : 0;

}

unsigned int kernel_pick(unsigned int itask, unsigned int index, struct message *message, unsigned int count, void *data)
{

    struct task *task = gettask(itask);

    if (task)
    {

        unsigned int source = task->inodes[index];

        if (source)
        {

            struct mailbox *mailbox = getmailbox(source);

            if (mailbox)
            {

                unsigned int status = mailbox_pick(mailbox, message, count, data);

                if (status == MESSAGE_RETRY)
                    task_signal(task, TASK_SIGNAL_BLOCK);

                return status;

            }

        }

    }

    return MESSAGE_FAILED;

}

unsigned int kernel_place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct noderow *snoderow = &noderows[source];
    struct noderow *tnoderow = &noderows[target];

    if (snoderow && tnoderow)
    {

        switch (event)
        {

        case EVENT_LINK:
            return kernel_link(&tnoderow->targets, snoderow->mailbox, snoderow->resource, snoderow->place) ? MESSAGE_OK : MESSAGE_FAILED;

        case EVENT_UNLINK:
            kernel_unlink(&tnoderow->targets, source);

            return MESSAGE_OK;

        }

        return (tnoderow->place) ? tnoderow->place(source, target, event, count, data) : MESSAGE_FAILED;

    }

    return MESSAGE_FAILED;

}

unsigned int kernel_placetask(unsigned int itask, unsigned int index, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct task *task = gettask(itask);

    if (!task->inodes[index])
        task->inodes[index] = kernel_link(0, pickmailbox(), &task->resource, placetask);

    return (task) ? kernel_place(task->inodes[index], target, event, count, data) : MESSAGE_FAILED;

}

void kernel_notify(unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct list *targets = &noderows[source].targets;
    struct list_item *noderowitem;

    spinlock_acquire(&targets->spinlock);

    for (noderowitem = targets->head; noderowitem; noderowitem = noderowitem->next)
    {

        struct noderow *noderow = noderowitem->data;

        kernel_place(source, encodenoderow(noderow), event, count, data);

    }

    spinlock_release(&targets->spinlock);

}

unsigned int kernel_createtask(void)
{

    struct list_item *taskrowitem = list_picktail(&deadtasks);

    if (taskrowitem)
    {

        struct taskrow *taskrow = taskrowitem->data;
        struct task *task = &taskrow->task;

        task_reset(task);

        if (task_transition(task, TASK_STATE_NEW))
            return encodetaskrow(taskrow);

    }

    return 0;

}

unsigned int kernel_loadtask(unsigned int itask, unsigned int ntask, unsigned int ip, unsigned int sp, unsigned int address)
{

    struct taskrow *taskrow = &taskrows[ntask];
    struct task *task = &taskrow->task;

    task->thread.ip = ip;
    task->thread.sp = sp;
    task->base = address;

    if (task->base)
    {

        struct binary_format *format = binary_findformat(task->base);

        if (format)
            task->thread.ip = format->findentry(task->base);

    }

    if (task->thread.ip)
    {

        if (task_transition(&taskrow->task, TASK_STATE_ASSIGNED))
        {

            assign(&taskrow->item);

            task->inodes[0] = kernel_link(0, pickmailbox(), &task->resource, placetask);

            return task->inodes[0];

        }

    }

    else
    {

        if (task_transition(&taskrow->task, TASK_STATE_DEAD))
            list_add(&deadtasks, &taskrow->item);

    }

    return 0;

}

void kernel_setup(unsigned int saddress, unsigned int ssize, unsigned int mbaddress, unsigned int mbsize)
{

    unsigned int i;

    core_init(&core0, 0, saddress + ssize);
    list_init(&freenodes);
    list_init(&deadtasks);
    list_init(&blockedtasks);
    list_init(&usednodes);
    list_init(&freemailboxes);
    list_init(&usedmailboxes);

    for (i = 1; i < KERNEL_MAILBOXES; i++)
    {

        struct mailboxrow *mailboxrow = &mailboxrows[i];

        mailbox_init(&mailboxrow->mailbox, (void *)(mbaddress + i * mbsize), mbsize);
        mailbox_register(&mailboxrow->mailbox);
        list_inititem(&mailboxrow->item, mailboxrow);
        list_add(&freemailboxes, &mailboxrow->item);

    }

    for (i = 1; i < KERNEL_TASKS; i++)
    {

        struct taskrow *taskrow = &taskrows[i];

        task_init(&taskrow->task);
        task_register(&taskrow->task);
        list_inititem(&taskrow->item, taskrow);
        list_add(&deadtasks, &taskrow->item);

    }

    for (i = 1; i < KERNEL_NODES; i++)
    {

        struct noderow *noderow = &noderows[i];

        list_inititem(&noderow->item, noderow);
        list_add(&freenodes, &noderow->item);

    }

}

