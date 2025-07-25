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
static struct list mailboxnodes;
static struct list modulenodes;
static struct list freetasks;
static struct list blockedtasks;
static struct list freemailboxes;
static struct list usedmailboxes;
static struct core *(*getcallback)(void);
static void (*assigncallback)(struct list_item *item);
static struct core core0;
static struct service mailboxservice;

static void *pickrow(struct list *from, struct list *to)
{

    struct list_item *item = list_pickhead(from);

    if (item)
    {

        list_add(to, item);

        return item->data;

    }

    return 0;

}

static struct noderow *getnoderow(unsigned int inode)
{

    return (inode && inode < KERNEL_NODES) ? &noderows[inode] : 0;

}

static struct taskrow *gettaskrow(unsigned int itask)
{

    return (itask && itask < KERNEL_TASKS) ? &taskrows[itask] : 0;

}

static struct mailboxrow *getmailboxrow(unsigned int imailbox)
{

    return (imailbox && imailbox < KERNEL_MAILBOXES) ? &mailboxrows[imailbox] : 0;

}

static unsigned int encodenoderow(struct noderow *noderow)
{

    return ((unsigned int)noderow - (unsigned int)noderows) / sizeof (struct noderow);

}

static unsigned int encodetaskrow(struct taskrow *taskrow)
{

    return ((unsigned int)taskrow - (unsigned int)taskrows) / sizeof (struct taskrow);

}

static unsigned int encodemailboxrow(struct mailboxrow *mailboxrow)
{

    return ((unsigned int)mailboxrow - (unsigned int)mailboxrows) / sizeof (struct mailboxrow);

}

static struct node *getnode(unsigned int inode)
{

    struct noderow *noderow = getnoderow(inode);

    return noderow ? &noderow->node : 0;

}

static struct task *gettask(unsigned int itask)
{

    struct taskrow *taskrow = gettaskrow(itask);

    return taskrow ? &taskrow->task : 0;

}

static struct mailbox *getmailbox(unsigned int imailbox)
{

    struct mailboxrow *mailboxrow = getmailboxrow(imailbox);

    return mailboxrow ? &mailboxrow->mailbox : 0;

}

static unsigned int addnode(struct list *nodes, char *name, struct resource *resource, struct service *service)
{

    struct noderow *noderow = pickrow(&freenodes, nodes);

    if (noderow)
    {

        node_reset(&noderow->node, name, resource, service);

        return encodenoderow(noderow);

    }

    return 0;

}

static void removenode(struct list *nodes, unsigned int inode)
{

    struct noderow *noderow = getnoderow(inode);

    if (noderow)
        list_move(&freenodes, nodes, &noderow->item);

}

static unsigned int addmailbox(unsigned int itask)
{

    struct mailboxrow *mailboxrow = pickrow(&freemailboxes, &usedmailboxes);

    if (mailboxrow)
    {

        struct mailbox *mailbox = &mailboxrow->mailbox;

        mailbox_reset(mailbox, itask, addnode(&mailboxnodes, 0, &mailbox->resource, &mailboxservice));

        return encodemailboxrow(mailboxrow);

    }

    return 0;

}

static void removemailbox(unsigned int imailbox)
{

    struct mailboxrow *mailboxrow = getmailboxrow(imailbox);

    if (mailboxrow)
    {

        removenode(&mailboxnodes, mailboxrow->mailbox.inode);
        list_move(&freemailboxes, &usedmailboxes, &mailboxrow->item);

    }

}

static void assign(struct list_item *item)
{

    if (assigncallback)
        assigncallback(item);
    else
        list_add(&core0.tasks, item);

}

static void checksignals(struct core *core)
{

    struct taskrow *taskrow = gettaskrow(core->itask);

    if (taskrow)
    {

        struct task *task = &taskrow->task;
        struct list_item *item = &taskrow->item;

        if (task->signals.kills)
        {

            if (task_transition(task, TASK_STATE_DEAD))
            {

                unsigned int i;

                for (i = 0; i < TASK_MAILBOXES; i++)
                {

                    if (task->imailbox[i])
                    {

                        removemailbox(task->imailbox[i]);

                        task->imailbox[i] = 0;

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

static unsigned int picknewtask(struct core *core)
{

    struct list_item *taskrowitem = list_pickhead(&core->tasks);

    if (taskrowitem)
    {

        struct taskrow *taskrow = taskrowitem->data;
        struct task *task = &taskrow->task;

        if (task_transition(task, TASK_STATE_RUNNING))
            return encodetaskrow(taskrow);

    }

    return 0;

}

static unsigned int service_pick(unsigned int source, struct message *message, unsigned int count, void *data)
{

    struct mailbox *mailbox = kernel_getnodeinterface(source);

    if (mailbox)
    {

        unsigned int status = mailbox_pick(mailbox, message, count, data);

        if (status == MESSAGE_RETRY)
            kernel_signal(mailbox->itask, TASK_SIGNAL_BLOCK);

        return status;

    }

    return MESSAGE_FAILED;

}

static unsigned int service_place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct mailbox *mailbox = kernel_getnodeinterface(target);

    if (mailbox)
    {

        unsigned int status = mailbox_place(mailbox, event, source, count, data);

        kernel_signal(mailbox->itask, TASK_SIGNAL_UNBLOCK);

        return status;

    }

    return MESSAGE_FAILED;

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

unsigned int kernel_addnode(char *name, struct resource *resource, struct service *service)
{

    return addnode(&modulenodes, name, resource, service);

}

void kernel_removenode(unsigned int inode)
{

    removenode(&modulenodes, inode);

}

unsigned int kernel_getchannelinode(unsigned int itask, unsigned int ichannel, unsigned int hack)
{

    struct task *task = gettask(itask);

    if (task)
    {

        struct mailbox *mailbox = getmailbox(task->imailbox[ichannel]);

        if (!mailbox && hack)
        {

            task->imailbox[ichannel] = addmailbox(itask);
            mailbox = getmailbox(task->imailbox[ichannel]);

        }

        return (mailbox) ? mailbox->inode : 0;

    }

    return 0;

}

unsigned int kernel_findinode(unsigned int namehash, unsigned int index)
{

    unsigned int n = 0;
    unsigned int i;

    for (i = 0; i < KERNEL_NODES; i++)
    {

        struct noderow *noderow = &noderows[i];

        if (noderow->node.namehash == namehash)
        {

            if (n == index)
                return encodenoderow(noderow);

            n++;

        }

    }

    return 0;

}

unsigned int kernel_linknode(unsigned int target, unsigned int source)
{

    struct node *snode = getnode(source);
    struct node *tnode = getnode(target);

    if (snode && tnode)
    {

        unsigned int inode = addnode(&tnode->links, 0, snode->resource, snode->service);

        if (inode)
            return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

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

    checksignals(core);
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

unsigned int kernel_pick(unsigned int source, struct message *message, unsigned int count, void *data)
{

    struct node *snode = getnode(source);

    return (snode) ? snode->service->pick(source, message, count, data) : MESSAGE_FAILED;

}

unsigned int kernel_place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct node *snode = getnode(source);
    struct node *tnode = getnode(target);

    return (snode && tnode) ? tnode->service->place(source, target, event, count, data) : MESSAGE_FAILED;

}

unsigned int kernel_announce(unsigned int inode, unsigned int namehash)
{

    struct node *node = getnode(inode);

    return (node) ? node->namehash = namehash : 0;

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

    struct list_item *taskrowitem = list_pickhead(&freetasks);

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

            task->imailbox[0] = addmailbox(ntask);

            if (task->imailbox[0])
            {

                struct mailbox *mailbox = getmailbox(task->imailbox[0]);

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

void kernel_setup(unsigned int saddress, unsigned int ssize, unsigned int mbaddress, unsigned int mbsize)
{

    unsigned int i;

    core_init(&core0, 0, saddress + ssize);
    list_init(&freenodes);
    list_init(&mailboxnodes);
    list_init(&modulenodes);
    list_init(&freetasks);
    list_init(&blockedtasks);
    list_init(&freemailboxes);
    list_init(&usedmailboxes);
    service_init(&mailboxservice, service_pick, service_place);
    service_register(&mailboxservice);

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

        node_init(&noderow->node);
        list_inititem(&noderow->item, noderow);
        list_add(&freenodes, &noderow->item);

    }

}

