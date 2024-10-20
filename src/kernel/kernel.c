#include <fudge.h>
#include "resource.h"
#include "node.h"
#include "debug.h"
#include "binary.h"
#include "mailbox.h"
#include "task.h"
#include "core.h"
#include "service.h"
#include "kernel.h"

static unsigned int mailboxcount;
static struct mailbox mailboxes[KERNEL_MAILBOXES];
static struct taskrow {struct task task; struct list_item item;} taskrows[KERNEL_TASKS];
static struct noderow {struct node node; struct list_item item;} noderows[KERNEL_NODES];
static struct list freenodes;
static struct list deadtasks;
static struct list blockedtasks;
static struct core *(*coreget)(void);
static void (*coreassign)(struct list_item *item);
static struct core core0;

static struct core *coreget0(void)
{

    return &core0;

}

static void coreassign0(struct list_item *item)
{

    list_add(&core0.tasks, item);

}

static struct task *gettask(unsigned int itask)
{

    return (itask && itask < KERNEL_TASKS) ? &taskrows[itask].task : 0;

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
                    coreassign(taskrowitem);

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

static unsigned int placetask(struct node *source, struct node *target, unsigned int event, unsigned int count, void *data)
{

    struct task *task = target->interface;
    struct message message;
    unsigned int status;

    message_init(&message, event, (unsigned int)source, count);

    status = mailbox_place(target->mailbox, &message, data);

    task_signal(task, TASK_SIGNAL_UNBLOCK);

    return status;

}

static unsigned int picknewtask(struct core *core)
{

    struct list_item *taskrowitem = list_picktail(&core->tasks);

    if (taskrowitem)
    {

        struct taskrow *taskrow = taskrowitem->data;
        struct task *task = &taskrow->task;

        if (task_transition(task, TASK_STATE_RUNNING))
            return task->id;

    }

    return 0;

}

struct core *kernel_getcore(void)
{

    return coreget();

}

void kernel_setcallback(struct core *(*get)(void), void (*assign)(struct list_item *item))
{

    coreget = get;
    coreassign = assign;

}

static unsigned int link(struct node *source, struct node *target)
{

    struct list_item *noderowitem = list_picktail(&freenodes);

    if (noderowitem)
    {

        struct noderow *noderow = noderowitem->data;
        struct node *node = &noderow->node;

        node_init(node, target->mailbox, target->interface, placetask);
        list_add(&source->children, noderowitem);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

static unsigned int unlink(struct node *source, struct node *target)
{

    struct list_item *noderowitem;
    struct list_item *next;

    spinlock_acquire(&source->children.spinlock);

    for (noderowitem = source->children.head; noderowitem; noderowitem = next)
    {

        struct noderow *noderow = noderowitem->data;
        struct node *node = &noderow->node;

        next = noderowitem->next;

        if (node->mailbox == target->mailbox)
        {

            list_remove_unsafe(&source->children, noderowitem);
            list_add(&freenodes, noderowitem);

        }

    }

    spinlock_release(&source->children.spinlock);

    return MESSAGE_OK;

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
    unsigned int status = mailbox_pick(task->node.mailbox, message, count, data);

    if (status == MESSAGE_RETRY)
        task_signal(task, TASK_SIGNAL_BLOCK);

    return status;

}

unsigned int kernel_place(struct node *source, struct node *target, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_LINK:
        return link(target, source);

    case EVENT_UNLINK:
        return unlink(target, source);

    }

    return (target->place) ? target->place(source, target, event, count, data) : 0;

}

unsigned int kernel_placetask(unsigned int itask, unsigned int index, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct task *task = gettask(itask);

    return (task && target) ? kernel_place(&task->node, (struct node *)target, event, count, data) : 0;

}

unsigned int kernel_find(unsigned int itask, unsigned int count, char *name)
{

    struct resource *resourceitem = 0;

    while ((resourceitem = resource_foreachtype(resourceitem, RESOURCE_SERVICE)))
    {

        struct service *service = resourceitem->data;
        unsigned int length = cstring_length(service->name);

        if (count >= length && buffer_match(name, service->name, length))
        {

            struct node *node = service->match(count - length, name + length);

            if (node)
                return (unsigned int)node;

        }

    }

    return 0;

}

void kernel_notify(struct node *source, unsigned int event, unsigned int count, void *data)
{

    struct list_item *noderowitem;

    spinlock_acquire(&source->children.spinlock);

    for (noderowitem = source->children.head; noderowitem; noderowitem = noderowitem->next)
    {

        struct noderow *noderow = noderowitem->data;
        struct node *node = &noderow->node;

        kernel_place(source, node, event, count, data);

    }

    spinlock_release(&source->children.spinlock);

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
            return task->id;

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
    task->node.place = placetask;

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

            mailbox_reset(task->node.mailbox);
            coreassign(&taskrow->item);

            return (unsigned int)&task->node;

        }

    }

    else
    {

        if (task_transition(&taskrow->task, TASK_STATE_DEAD))
        {

            list_add(&deadtasks, &taskrow->item);

            return 0;

        }

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

    for (i = 1; i < KERNEL_MAILBOXES; i++)
    {

        mailbox_init(&mailboxes[i], (void *)(mbaddress + i * mbsize), mbsize);
        mailbox_register(&mailboxes[i]);

    }

    for (i = 1; i < KERNEL_TASKS; i++)
    {

        struct taskrow *taskrow = &taskrows[i];

        task_init(&taskrow->task, i);
        task_register(&taskrow->task);
        list_inititem(&taskrow->item, taskrow);
        list_add(&deadtasks, &taskrow->item);

        /* Allow many nodes */
        node_init(&taskrow->task.node, &mailboxes[++mailboxcount], &taskrow->task, placetask);

    }

    for (i = 0; i < KERNEL_NODES; i++)
    {

        struct noderow *noderow = &noderows[i];

        list_inititem(&noderow->item, noderow);
        list_add(&freenodes, &noderow->item);

    }

    kernel_setcallback(coreget0, coreassign0);

}

