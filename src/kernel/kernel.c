#include <fudge.h>
#include "resource.h"
#include "debug.h"
#include "binary.h"
#include "mailbox.h"
#include "task.h"
#include "node.h"
#include "core.h"
#include "service.h"
#include "kernel.h"

static struct taskrow {struct list_item item; struct task task;} taskrows[KERNEL_TASKS];
static struct mailboxrow {struct list_item item; struct mailbox mailbox;} mailboxrows[KERNEL_MAILBOXES];
static struct noderow {struct list_item item; struct node node;} noderows[KERNEL_NODES];
static struct list freenodes;
static struct list usednodes;
static struct list freetasks;
static struct list blockedtasks;
static struct list freemailboxes;
static struct list usedmailboxes;
static struct core *(*getcallback)(void);
static void (*assigncallback)(struct list_item *item);
static struct core core0;

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

static struct noderow *getnoderow(unsigned int inode)
{

    return (inode && inode < KERNEL_NODES) ? &noderows[inode] : 0;

}

static struct mailboxrow *getmailboxrow(unsigned int imailbox)
{

    return (imailbox && imailbox < KERNEL_MAILBOXES) ? &mailboxrows[imailbox] : 0;

}

static struct taskrow *gettaskrow(unsigned int itask)
{

    return (itask && itask < KERNEL_TASKS) ? &taskrows[itask] : 0;

}

static unsigned int encodenoderow(struct noderow *noderow)
{

    return ((unsigned int)noderow - (unsigned int)noderows) / sizeof (struct noderow);

}

static unsigned int encodemailboxrow(struct mailboxrow *mailboxrow)
{

    return ((unsigned int)mailboxrow - (unsigned int)mailboxrows) / sizeof (struct mailboxrow);

}

static unsigned int encodetaskrow(struct taskrow *taskrow)
{

    return ((unsigned int)taskrow - (unsigned int)taskrows) / sizeof (struct taskrow);

}

static struct node *getnode(unsigned int inode)
{

    struct noderow *noderow = getnoderow(inode);

    return noderow ? &noderow->node : 0;

}

static struct mailbox *getmailbox(unsigned int imailbox)
{

    struct mailboxrow *mailboxrow = getmailboxrow(imailbox);

    return mailboxrow ? &mailboxrow->mailbox : 0;

}

static struct task *gettask(unsigned int itask)
{

    struct taskrow *taskrow = gettaskrow(itask);

    return taskrow ? &taskrow->task : 0;

}

static unsigned int addnode(struct list *nodes, struct resource *resource, unsigned int imailbox, unsigned int (*place)(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data))
{

    struct noderow *noderow = pickrow(&freenodes, nodes);

    if (noderow)
    {

        node_init(&noderow->node, resource, imailbox, place);

        return encodenoderow(noderow);

    }

    return 0;

}

static void removenode(struct list *nodes, unsigned int inode)
{

    struct noderow *noderow = getnoderow(inode);

    if (noderow)
        returnrow(nodes, &freenodes, &noderow->item);

}

static void assign(struct list_item *item)
{

    if (assigncallback)
        assigncallback(item);
    else
        list_add(&core0.tasks, item);

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
        {

            unsigned int i;

            for (i = 0; i < TASK_NODES; i++)
            {

                if (task->inodes[i])
                {

                    struct node *node = getnode(task->inodes[i]);

                    if (node && node->imailbox)
                    {

                        struct mailboxrow *mailboxrow = getmailboxrow(node->imailbox);

                        if (mailboxrow)
                            returnrow(&usedmailboxes, &freemailboxes, &mailboxrow->item);

                    }

                    kernel_removenode(task->inodes[i]);

                }

            }

            list_add(&freetasks, item);

        }

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

static unsigned int placemailbox(unsigned int source, unsigned int imailbox, unsigned int event, unsigned int count, void *data)
{

    struct mailbox *mailbox = getmailbox(imailbox);

    if (mailbox)
    {

        struct message message;

        message_init(&message, event, source, count);

        return mailbox_place(mailbox, &message, data);

    }

    return MESSAGE_FAILED;

}

static unsigned int placetask(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct node *node = getnode(target);

    if (node)
    {

        struct task *task = node->resource->data;

        if (task)
        {

            unsigned int status = placemailbox(source, node->imailbox, event, count, data);

            task_signal(task, TASK_SIGNAL_UNBLOCK);

            return status;

        }

    }

    return MESSAGE_FAILED;

}

static unsigned int picknewmailbox(void)
{

    struct mailboxrow *mailboxrow = pickrow(&freemailboxes, &usedmailboxes);

    if (mailboxrow)
    {

        struct mailbox *mailbox = &mailboxrow->mailbox;

        mailbox_reset(mailbox);

        return encodemailboxrow(mailboxrow);

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

void *kernel_getnodeinterface(unsigned int inode)
{

    struct node *node = getnode(inode);

    return (node) ? node->resource->data : 0;

}

struct task_thread *kernel_gettaskthread(unsigned int itask)
{

    struct task *task = gettask(itask);

    return (task) ? &task->thread : 0;

}

struct core *kernel_getcore(void)
{

    return (getcallback) ? getcallback() : &core0;

}

unsigned int kernel_addnode(struct resource *resource, unsigned int (*place)(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data))
{

    return addnode(&usednodes, resource, 0, place);

}

unsigned int kernel_linknode(unsigned int target, unsigned int source)
{

    struct node *snode = getnode(source);
    struct node *tnode = getnode(target);

    if (snode && tnode)
    {

        unsigned int inode = addnode(&tnode->links, snode->resource, snode->imailbox, snode->place);

        if (inode)
            return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

void kernel_removenode(unsigned int inode)
{

    removenode(&usednodes, inode);

}

unsigned int kernel_unlinknode(unsigned int target, unsigned int source)
{

    struct node *snode = getnode(source);
    struct node *tnode = getnode(target);

    if (snode && tnode)
    {

        removenode(&tnode->links, source);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

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

        struct binary_format *format = binary_findformat(task->address);

        return (format) ? format->findbase(task->address, address) : 0;

    }

    return 0;

}

unsigned int kernel_loadprogram(unsigned int itask)
{

    struct task *task = gettask(itask);

    if (task)
    {

        struct binary_format *format = binary_findformat(task->address);

        return (format) ? format->copyprogram(task->address) : 0;

    }

    return 0;

}

void kernel_signal(unsigned int itask, unsigned int signal)
{

    struct task *task = gettask(itask);

    if (task)
        task_signal(task, signal);

}

unsigned int kernel_pick(unsigned int itask, unsigned int ichannel, struct message *message, unsigned int count, void *data)
{

    struct task *task = gettask(itask);

    if (task)
    {

        unsigned int source = task->inodes[ichannel];

        if (source)
        {

            struct node *node = getnode(source);

            if (node)
            {

                struct mailbox *mailbox = getmailbox(node->imailbox);

                if (mailbox)
                {

                    unsigned int status = mailbox_pick(mailbox, message, count, data);

                    if (status == MESSAGE_RETRY)
                        task_signal(task, TASK_SIGNAL_BLOCK);

                    return status;

                }

            }

        }

    }

    return MESSAGE_FAILED;

}

unsigned int kernel_place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct node *snode = getnode(source);
    struct node *tnode = getnode(target);

    if (snode && tnode)
        return tnode->place(source, target, event, count, data);

    return MESSAGE_FAILED;

}

unsigned int kernel_placetask(unsigned int itask, unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct task *task = gettask(itask);

    if (task)
    {

        if (!task->inodes[ichannel])
        {

            unsigned int imailbox = picknewmailbox();

            if (imailbox)
                task->inodes[ichannel] = addnode(&usednodes, &task->resource, imailbox, placetask);

        }

        return kernel_place(task->inodes[ichannel], target, event, count, data);

    }

    return MESSAGE_FAILED;

}

void kernel_notify(unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct node *snode = getnode(source);

    if (snode)
    {

        struct list *links = &snode->links;
        struct list_item *noderowitem;

        spinlock_acquire(&links->spinlock);

        for (noderowitem = links->head; noderowitem; noderowitem = noderowitem->next)
        {

            struct noderow *tnoderow = noderowitem->data;

            kernel_place(source, encodenoderow(tnoderow), event, count, data);

        }

        spinlock_release(&links->spinlock);

    }

}

unsigned int kernel_createtask(void)
{

    struct list_item *taskrowitem = list_picktail(&freetasks);

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

    struct taskrow *taskrow = gettaskrow(ntask);

    if (taskrow)
    {

        struct task *task = &taskrow->task;

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

            if (task_transition(task, TASK_STATE_ASSIGNED))
                assign(&taskrow->item);

        }

        else
        {

            if (task_transition(task, TASK_STATE_DEAD))
                list_add(&freetasks, &taskrow->item);

        }

        if (task->thread.ip)
        {

            unsigned int imailbox = picknewmailbox();

            if (imailbox)
            {

                task->inodes[0] = addnode(&usednodes, &task->resource, imailbox, placetask);

                return task->inodes[0];

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

void kernel_setup(unsigned int saddress, unsigned int ssize, unsigned int mbaddress, unsigned int mbsize)
{

    unsigned int i;

    core_init(&core0, 0, saddress + ssize);
    list_init(&freenodes);
    list_init(&usednodes);
    list_init(&freetasks);
    list_init(&blockedtasks);
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
        list_add(&freetasks, &taskrow->item);

    }

    for (i = 1; i < KERNEL_NODES; i++)
    {

        struct noderow *noderow = &noderows[i];

        list_inititem(&noderow->item, noderow);
        list_add(&freenodes, &noderow->item);

    }

}

