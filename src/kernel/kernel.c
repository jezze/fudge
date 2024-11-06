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
static struct taskrow {unsigned int id; struct task task; struct list_item item;} taskrows[KERNEL_TASKS];
static struct noderow {unsigned int id; struct node node; struct list_item item;} noderows[KERNEL_NODES];
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

    struct task *task = target->resource->data;
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
            return taskrow->id;

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

struct node *kernel_getnode(struct list *list, unsigned int index)
{

    struct list_item *noderowitem = list->head;

    if (noderowitem)
    {

        struct noderow *noderow = noderowitem->data;

        return &noderow->node;

    }

    return 0;

}

struct node *kernel_link(struct list *nodes, struct mailbox *mailbox, struct resource *resource, unsigned int (*place)(struct node *source, struct node *target, unsigned int event, unsigned int count, void *data))
{

    struct list_item *noderowitem = list_picktail(&freenodes);

    if (noderowitem)
    {

        struct noderow *noderow = noderowitem->data;

        node_init(&noderow->node, mailbox, resource, place);
        list_add(nodes, noderowitem);

        return &noderow->node;

    }

    return 0;

}

void kernel_unlink(struct list *targets, struct node *target)
{

    struct list_item *noderowitem;
    struct list_item *next;

    spinlock_acquire(&targets->spinlock);

    for (noderowitem = targets->head; noderowitem; noderowitem = next)
    {

        struct noderow *noderow = noderowitem->data;
        struct node *node = &noderow->node;

        next = noderowitem->next;

        if (node->mailbox == target->mailbox)
        {

            list_remove_unsafe(targets, noderowitem);
            list_add(&freenodes, noderowitem);

        }

    }

    spinlock_release(&targets->spinlock);

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

        struct node *node = kernel_getnode(&task->resource.sources, index);

        if (node)
        {

            unsigned int status = mailbox_pick(node->mailbox, message, count, data);

            if (status == MESSAGE_RETRY)
                task_signal(task, TASK_SIGNAL_BLOCK);

            return status;

        }

    }

    return MESSAGE_FAILED;

}

unsigned int kernel_place(unsigned int index, struct list *sources, struct node *target, unsigned int event, unsigned int count, void *data)
{

    struct node *source = kernel_getnode(sources, index);

    if (source)
    {

        switch (event)
        {

        case EVENT_LINK:
            return kernel_link(&target->resource->targets, source->mailbox, source->resource, source->place) ? MESSAGE_OK : MESSAGE_FAILED;

        case EVENT_UNLINK:
            kernel_unlink(&target->resource->targets, source);

            return MESSAGE_OK;

        }

        return (target->place) ? target->place(source, target, event, count, data) : 0;

    }

    return 0;

}

unsigned int kernel_placetask(unsigned int itask, unsigned int index, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct task *task = gettask(itask);

    return (task && target) ? kernel_place(index, &task->resource.sources, (struct node *)target, event, count, data) : 0;

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

void kernel_notify(unsigned int index, struct list *sources, struct list *targets, unsigned int event, unsigned int count, void *data)
{

    struct list_item *noderowitem;

    spinlock_acquire(&targets->spinlock);

    for (noderowitem = targets->head; noderowitem; noderowitem = noderowitem->next)
    {

        struct noderow *noderow = noderowitem->data;
        struct node *target = &noderow->node;

        kernel_place(index, sources, target, event, count, data);

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
        kernel_link(&task->resource.sources, &mailboxes[++mailboxcount], &task->resource, placetask);

        if (task_transition(task, TASK_STATE_NEW))
            return taskrow->id;

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

            struct node *node = kernel_getnode(&task->resource.sources, 0);

            mailbox_reset(node->mailbox);
            coreassign(&taskrow->item);

            return (unsigned int)node;

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

        taskrow->id = i;

        task_init(&taskrow->task);
        task_register(&taskrow->task);
        list_inititem(&taskrow->item, taskrow);
        list_add(&deadtasks, &taskrow->item);

    }

    for (i = 0; i < KERNEL_NODES; i++)
    {

        struct noderow *noderow = &noderows[i];

        noderow->id = i;

        list_inititem(&noderow->item, noderow);
        list_add(&freenodes, &noderow->item);

    }

    kernel_setcallback(coreget0, coreassign0);

}

