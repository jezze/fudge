#include <fudge.h>
#include "resource.h"
#include "debug.h"
#include "binary.h"
#include "mailbox.h"
#include "task.h"
#include "core.h"
#include "link.h"
#include "service.h"
#include "descriptor.h"
#include "kernel.h"

struct channel
{

    unsigned int target;
    struct service *service;
    unsigned short uniqueness;
    unsigned int (*place)(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data);

};

static struct channel channels[KERNEL_CHANNELS];
static struct mailbox mailboxes[KERNEL_MAILBOXES];
static struct taskrow {struct task task; struct descriptor descriptors[KERNEL_DESCRIPTORS]; struct list_item item;} taskrows[KERNEL_TASKS];
static struct linkrow {struct link link; struct list_item item;} linkrows[KERNEL_LINKS];
static struct list freelinks;
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

static void unblocktasks(void)
{

    struct list_item *taskitem;
    struct list_item *next;

    spinlock_acquire(&blockedtasks.spinlock);

    for (taskitem = blockedtasks.head; taskitem; taskitem = next)
    {

        struct taskrow *taskrow = taskitem->data;
        struct task *task = &taskrow->task;

        next = taskitem->next;

        if (task->signals.unblocks)
        {

            if (task_transition(task, TASK_STATE_UNBLOCKED))
            {

                list_remove_unsafe(&blockedtasks, taskitem);

                if (task_transition(task, TASK_STATE_ASSIGNED))
                    coreassign(taskitem);

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

static unsigned int placetask(unsigned int id, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    if (id)
    {

        struct mailbox *mailbox = &mailboxes[id];
        struct message message;
        unsigned int c;

        message_init(&message, event, source, count);

        c = mailbox_place(mailbox, &message, data);

        kernel_signal(id, TASK_SIGNAL_UNBLOCK);

        return c;

    }

    return 0;

}

unsigned int picknewtask(struct core *core)
{

    struct list_item *taskitem = list_picktail(&core->tasks);

    if (taskitem)
    {

        struct taskrow *taskrow = taskitem->data;
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

void kernel_addlink(struct list *list, unsigned int target, unsigned int source)
{

    struct list_item *linkitem = list_picktail(&freelinks);

    if (linkitem)
    {

        struct linkrow *linkrow = linkitem->data;
        struct link *link = &linkrow->link;

        link->source = source;
        link->target = target;

        list_add(list, linkitem);

    }

}

void kernel_removelink(struct list *list, unsigned int target)
{

    struct list_item *current;
    struct list_item *next;

    spinlock_acquire(&list->spinlock);

    for (current = list->head; current; current = next)
    {

        struct linkrow *linkrow = current->data;
        struct link *link = &linkrow->link;

        next = current->next;

        if (link->target == target)
        {

            list_remove_unsafe(list, current);
            list_add(&freelinks, current);

        }

    }

    spinlock_release(&list->spinlock);

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

    struct taskrow *taskrow = &taskrows[itask];
    struct binary_format *format = binary_findformat(&taskrow->task.node);

    return (format) ? format->findbase(&taskrow->task.node, address) : 0;

}

unsigned int kernel_loadprogram(unsigned int itask)
{

    struct taskrow *taskrow = &taskrows[itask];
    struct binary_format *format = binary_findformat(&taskrow->task.node);

    return (format) ? format->copyprogram(&taskrow->task.node) : 0;

}

void kernel_signal(unsigned int itask, unsigned int signal)
{

    struct taskrow *taskrow = &taskrows[itask];

    task_signal(&taskrow->task, signal);

}

struct task_thread *kernel_getthread(unsigned int itask)
{

    struct taskrow *taskrow = &taskrows[itask];

    return &taskrow->task.thread;

}

struct descriptor *kernel_getdescriptor(unsigned int itask, unsigned int idescriptor)
{

    struct taskrow *taskrow = &taskrows[itask];

    return &taskrow->descriptors[(idescriptor & (KERNEL_DESCRIPTORS - 1))];

}

void kernel_setdescriptor(unsigned int itask, unsigned int idescriptor, struct service *service, unsigned int id)
{

    struct descriptor *descriptor = kernel_getdescriptor(itask, idescriptor);

    descriptor->service = service;
    descriptor->id = id;

}

void kernel_copydescriptor(unsigned int itask, unsigned int idescriptor, unsigned int iptask, unsigned int ipdescriptor)
{

    struct descriptor *descriptor = kernel_getdescriptor(itask, idescriptor);
    struct descriptor *pdescriptor = kernel_getdescriptor(iptask, ipdescriptor);

    descriptor->service = pdescriptor->service;
    descriptor->id = pdescriptor->id;

}

unsigned int kernel_pick(unsigned int source, struct message *message, unsigned int count, void *data)
{

    struct mailbox *mailbox = &mailboxes[source];
    unsigned int c = mailbox_pick(mailbox, message, count, data);

    if (!c)
        kernel_signal(source, TASK_SIGNAL_BLOCK);

    return c;

}

unsigned int kernel_place(unsigned int source, unsigned int ichannel, unsigned int event, unsigned int count, void *data)
{

    struct channel *channel = (ichannel < KERNEL_CHANNELS) ? &channels[ichannel] : 0;

    return (channel) ? channel->place(channel->target, source, event, count, data) : 0;

}

void kernel_announce(unsigned short index, unsigned int target, struct service *service, unsigned int (*place)(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data))
{

    struct channel *channel = &channels[index];

    channel->target = target;
    channel->place = (place) ? place : placetask;
    channel->service = service;
    channel->uniqueness++;

}

void kernel_notify(struct list *links, unsigned int event, unsigned int count, void *data)
{

    struct list_item *current;

    spinlock_acquire(&links->spinlock);

    for (current = links->head; current; current = current->next)
    {

        struct link *link = current->data;

        kernel_place(link->source, link->target, event, count, data);

    }

    spinlock_release(&links->spinlock);

}

unsigned int kernel_createtask(void)
{

    struct list_item *taskitem = list_picktail(&deadtasks);

    if (taskitem)
    {

        struct taskrow *taskrow = taskitem->data;
        struct task *task = &taskrow->task;
        struct mailbox *mailbox = &mailboxes[task->id];
        unsigned int i;

        task_reset(task);
        mailbox_reset(mailbox);

        for (i = 0; i < KERNEL_DESCRIPTORS; i++)
            descriptor_reset(&taskrow->descriptors[i]);

        if (task_transition(task, TASK_STATE_NEW))
            return task->id;

    }

    return 0;

}

unsigned int kernel_loadtask(unsigned int itask, unsigned int ip, unsigned int sp, unsigned int ichannel, unsigned int id, unsigned int address)
{

    struct taskrow *taskrow = &taskrows[itask];
    struct task *task = &taskrow->task;

    task->thread.ip = ip;
    task->thread.sp = sp;
    task->node.address = address;

    if (ichannel && id)
    {

        struct channel *channel = (ichannel < KERNEL_CHANNELS) ? &channels[ichannel] : 0;

        if (channel)
        {

            kernel_setdescriptor(itask, FILE_PW, channel->service, channel->service->root());
            kernel_setdescriptor(itask, FILE_PP, channel->service, id);

        }

    }

    if (task->node.address)
    {

        struct binary_format *format = binary_findformat(&task->node);

        if (format)
            task->thread.ip = format->findentry(&task->node);

    }

    if (task->thread.ip)
    {

        if (task_transition(&taskrow->task, TASK_STATE_ASSIGNED))
        {

            coreassign(&taskrow->item);

            return itask;

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
    list_init(&freelinks);
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
        unsigned int j;

        task_init(&taskrow->task, i);
        task_register(&taskrow->task);
        list_inititem(&taskrow->item, taskrow);
        list_add(&deadtasks, &taskrow->item);

        for (j = 0; j < KERNEL_DESCRIPTORS; j++)
            descriptor_init(&taskrow->descriptors[j]);

        kernel_announce(i, i, 0, placetask);

    }

    for (i = 0; i < KERNEL_LINKS; i++)
    {

        struct linkrow *linkrow = &linkrows[i];

        link_init(&linkrow->link);
        list_inititem(&linkrow->item, linkrow);
        list_add(&freelinks, &linkrow->item);

    }

    kernel_setcallback(coreget0, coreassign0);

}

