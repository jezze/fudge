#include <fudge.h>
#include "resource.h"
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

    void *interface;
    struct list links;
    unsigned int (*place)(void *interface, unsigned int ichannel, unsigned int source, unsigned int event, unsigned int count, void *data);

};

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

static unsigned int placetask(void *interface, unsigned int ichannel, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct channel *channel = getchannel(ichannel);

    if (channel)
    {

        struct task *task = interface;
        struct mailbox *mailbox = &mailboxes[task->id];
        struct message message;
        unsigned int status;

        message_init(&message, event, source, count);

        status = mailbox_place(mailbox, &message, data);

        kernel_signal(task->id, TASK_SIGNAL_UNBLOCK);

        return status;

    }

    return MESSAGE_FAILED;

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

unsigned int kernel_link(unsigned int ichannel, unsigned int target)
{

    struct channel *channel = getchannel(ichannel);

    if (channel)
    {

        struct list_item *linkitem = list_picktail(&freelinks);

        if (linkitem)
        {

            struct linkrow *linkrow = linkitem->data;
            struct link *link = &linkrow->link;

            link_init(link, target);
            list_add(&channel->links, linkitem);

            return MESSAGE_OK;

        }

    }

    return MESSAGE_FAILED;

}

unsigned int kernel_unlink(unsigned int ichannel, unsigned int target)
{

    struct channel *channel = getchannel(ichannel);

    if (channel)
    {

        struct list_item *current;
        struct list_item *next;

        spinlock_acquire(&channel->links.spinlock);

        for (current = channel->links.head; current; current = next)
        {

            struct linkrow *linkrow = current->data;
            struct link *link = &linkrow->link;

            next = current->next;

            if (link->target == target)
            {

                list_remove_unsafe(&channel->links, current);
                list_add(&freelinks, current);

            }

        }

        spinlock_release(&channel->links.spinlock);

    }

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

unsigned int kernel_pick(unsigned int source, struct message *message, unsigned int count, void *data)
{

    struct mailbox *mailbox = &mailboxes[source];
    unsigned int status = mailbox_pick(mailbox, message, count, data);

    if (status == MESSAGE_RETRY)
        kernel_signal(source, TASK_SIGNAL_BLOCK);

    return status;

}

unsigned int kernel_place(unsigned int source, unsigned int ichannel, unsigned int event, unsigned int count, void *data)
{

    struct channel *channel = getchannel(ichannel);

    return (channel) ? channel->place(channel->interface, ichannel, source, event, count, data) : 0;

}

unsigned int kernel_find(unsigned int source, unsigned int count, char *name)
{

    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_SERVICE)))
    {

        struct service *service = current->data;
        unsigned int channel = service->match(count, name);

        if (channel)
            return channel;

    }

    return 0;

}

void kernel_announce(unsigned int ichannel, void *interface, unsigned int (*place)(void *interface, unsigned int ichannel, unsigned int source, unsigned int event, unsigned int count, void *data))
{

    struct channel *channel = getchannel(ichannel);

    if (channel)
    {

        list_init(&channel->links);

        channel->interface = interface;
        channel->place = place;

    }

}

void kernel_announce2(unsigned int ichannel, unsigned int itask)
{

    struct taskrow *taskrow = &taskrows[itask];

    kernel_announce(ichannel, &taskrow->task, placetask);

}

void kernel_unannounce(unsigned int ichannel)
{

    struct channel *channel = getchannel(ichannel);

    if (channel)
    {

        list_init(&channel->links);

        channel->interface = 0;
        channel->place = 0;

    }

}

void kernel_notify(unsigned int ichannel, unsigned int event, unsigned int count, void *data)
{

    struct channel *channel = getchannel(ichannel);

    if (channel)
    {

        struct list_item *current;

        spinlock_acquire(&channel->links.spinlock);

        for (current = channel->links.head; current; current = current->next)
        {

            struct linkrow *linkrow = current->data;
            struct link *link = &linkrow->link;

            kernel_place(ichannel, link->target, event, count, data);

        }

        spinlock_release(&channel->links.spinlock);

    }

}

unsigned int kernel_createtask(void)
{

    struct list_item *taskitem = list_picktail(&deadtasks);

    if (taskitem)
    {

        struct taskrow *taskrow = taskitem->data;
        struct task *task = &taskrow->task;
        struct mailbox *mailbox = &mailboxes[task->id];

        task_reset(task);
        mailbox_reset(mailbox);

        if (task_transition(task, TASK_STATE_NEW))
            return task->id;

    }

    return 0;

}

unsigned int kernel_loadtask(unsigned int itask, unsigned int ip, unsigned int sp, unsigned int address)
{

    struct taskrow *taskrow = &taskrows[itask];
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

        task_init(&taskrow->task, i);
        task_register(&taskrow->task);
        list_inititem(&taskrow->item, taskrow);
        list_add(&deadtasks, &taskrow->item);
        kernel_announce(i, &taskrow->task, placetask);

    }

    for (i = 0; i < KERNEL_LINKS; i++)
    {

        struct linkrow *linkrow = &linkrows[i];

        link_init(&linkrow->link, 0);
        list_inititem(&linkrow->item, linkrow);
        list_add(&freelinks, &linkrow->item);

    }

    kernel_setcallback(coreget0, coreassign0);

}

