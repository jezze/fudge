#include <fudge.h>
#include "resource.h"
#include "node.h"
#include "debug.h"
#include "binary.h"
#include "mailbox.h"
#include "task.h"
#include "core.h"
#include "link.h"
#include "service.h"
#include "kernel.h"

struct channel
{

    struct node *target;

};

static unsigned int channelcount;
static unsigned int mailboxcount;
static struct channel channels[KERNEL_CHANNELS];
static struct mailbox mailboxes[KERNEL_MAILBOXES];
static struct taskrow {struct task task; struct list_item item;} taskrows[KERNEL_TASKS];
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

static struct channel *getchannel(unsigned int ichannel)
{

    return (ichannel && ichannel < KERNEL_CHANNELS) ? &channels[ichannel] : 0;

}

static struct task *gettask(unsigned int itask)
{

    return (itask && itask < KERNEL_TASKS) ? &taskrows[itask].task : 0;

}

static struct link *getlink(struct node *node, unsigned int index)
{

    struct list_item *current;
    unsigned int i = 0;

    for (current = node->links.head; current; current = current->next)
    {

        if (i == index)
        {

            struct link *link = current->data;

            return link;

        }

    }

    return 0;

}

static struct mailbox *getmailbox(struct node *node, unsigned int index)
{

    struct link *link = getlink(node, index);

    return (link) ? link->mailbox : 0;

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

static unsigned int placetask(struct node *source, struct node *target, unsigned int event, unsigned int count, void *data)
{

    struct task *task = target->interface;
    struct mailbox *mailbox = getmailbox(&task->node, 0);
    struct message message;
    unsigned int status;

    message_init(&message, event, source->ichannel, count);

    status = mailbox_place(mailbox, &message, data);

    task_signal(task, TASK_SIGNAL_UNBLOCK);

    return status;

}

static unsigned int picknewtask(struct core *core)
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

static unsigned int link(struct node *source, struct node *target, struct mailbox *mailbox)
{

    struct list_item *linkitem = list_picktail(&freelinks);

    if (linkitem)
    {

        struct linkrow *linkrow = linkitem->data;
        struct link *link = &linkrow->link;

        link_init(link, target, mailbox);
        list_add(&source->links, linkitem);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

static unsigned int unlink(struct node *source, struct node *target)
{

    struct list_item *current;
    struct list_item *next;

    spinlock_acquire(&source->links.spinlock);

    for (current = source->links.head; current; current = next)
    {

        struct linkrow *linkrow = current->data;
        struct link *link = &linkrow->link;

        next = current->next;

        if (link->target == target)
        {

            list_remove_unsafe(&source->links, current);
            list_add(&freelinks, current);

        }

    }

    spinlock_release(&source->links.spinlock);

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

unsigned int kernel_pick(unsigned int itask, unsigned int ichannel, struct message *message, unsigned int count, void *data)
{

    struct task *task = gettask(itask);
    struct mailbox *mailbox = getmailbox(&task->node, 0);
    unsigned int status = mailbox_pick(mailbox, message, count, data);

    if (status == MESSAGE_RETRY)
        task_signal(task, TASK_SIGNAL_BLOCK);

    return status;

}

unsigned int kernel_place(struct node *source, struct node *target, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_LINK:
        return link(target, source, getmailbox(source->interface, 0));

    case EVENT_UNLINK:
        return unlink(target, source);

    }

    return (target && target->place) ? target->place(source, target, event, count, data) : 0;

}

unsigned int kernel_placetask(unsigned int itask, unsigned int ichannel, unsigned int event, unsigned int count, void *data)
{

    struct task *task = gettask(itask);
    struct channel *channel = getchannel(ichannel);

    return (channel) ? kernel_place(&task->node, channel->target, event, count, data) : 0;

}

unsigned int kernel_find(unsigned int itask, unsigned int count, char *name)
{

    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_SERVICE)))
    {

        struct service *service = current->data;
        unsigned int length = cstring_length(service->name);

        if (count >= length && buffer_match(name, service->name, length))
        {

            struct node *node = service->match(count - length, name + length);

            if (node)
                return node->ichannel;

        }

    }

    return 0;

}

unsigned int kernel_announce(struct node *target)
{

    unsigned int ichannel = ++channelcount;
    struct channel *channel = getchannel(ichannel);

    if (channel)
    {

        channel->target = target;
        channel->target->ichannel = ichannel;

    }

    return ichannel;

}

void kernel_unannounce(struct node *target)
{

}

void kernel_unannouncetask(unsigned int itask)
{

    struct task *task = gettask(itask);

    kernel_unannounce(&task->node);

}

void kernel_notify(struct node *source, unsigned int event, unsigned int count, void *data)
{

    struct list_item *current;

    spinlock_acquire(&source->links.spinlock);

    for (current = source->links.head; current; current = current->next)
    {

        struct linkrow *linkrow = current->data;
        struct link *link = &linkrow->link;

        kernel_place(source, link->target, event, count, data);

    }

    spinlock_release(&source->links.spinlock);

}

unsigned int kernel_createtask(void)
{

    struct list_item *taskitem = list_picktail(&deadtasks);

    if (taskitem)
    {

        struct taskrow *taskrow = taskitem->data;
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

            if (itask)
            {

                struct taskrow *parentrow = &taskrows[itask];
                struct task *parent = &parentrow->task;

                link(&parent->node, &task->node, &mailboxes[++mailboxcount]);
                link(&task->node, &parent->node, &mailboxes[++mailboxcount]);

            }

            else
            {

                link(&task->node, &task->node, &mailboxes[++mailboxcount]);

            }

            mailbox_reset(getmailbox(&task->node, 0));
            kernel_announce(&task->node);
            coreassign(&taskrow->item);

            return task->node.ichannel;

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

        task_init(&taskrow->task, i);
        task_register(&taskrow->task);
        list_inititem(&taskrow->item, taskrow);
        list_add(&deadtasks, &taskrow->item);

    }

    for (i = 0; i < KERNEL_LINKS; i++)
    {

        struct linkrow *linkrow = &linkrows[i];

        list_inititem(&linkrow->item, linkrow);
        list_add(&freelinks, &linkrow->item);

    }

    kernel_setcallback(coreget0, coreassign0);

}

