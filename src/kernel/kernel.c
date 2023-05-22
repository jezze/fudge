#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "mailbox.h"
#include "task.h"
#include "core.h"
#include "link.h"
#include "service.h"
#include "descriptor.h"
#include "kernel.h"

struct taskdata
{

    struct task task;
    struct mailbox mailbox;
    struct descriptor descriptors[KERNEL_DESCRIPTORS];
    struct list_item item;

};

struct linkdata
{

    struct link link;
    struct list_item item;

};

static struct taskdata taskdata[KERNEL_TASKS];
static struct linkdata linkdata[KERNEL_LINKS];
static struct list freelinks;
static struct list deadtasks;
static struct list blockedtasks;
static struct core *(*coreget)(void);
static void (*coreassign)(struct list_item *item);

static void unblocktasks(void)
{

    struct list_item *taskitem;
    struct list_item *next;

    spinlock_acquire(&blockedtasks.spinlock);

    for (taskitem = blockedtasks.head; taskitem; taskitem = next)
    {

        struct task *task = taskitem->data;
        struct mailbox *mailbox = &taskdata[task->id].mailbox;

        next = taskitem->next;

        spinlock_acquire(&mailbox->spinlock);

        if (ring_count(&mailbox->ring))
        {

            if (task_transition(task, TASK_STATE_UNBLOCKED))
                list_remove_unsafe(&blockedtasks, taskitem);

            if (task_transition(task, TASK_STATE_ASSIGNED))
                coreassign(taskitem);

        }

        spinlock_release(&mailbox->spinlock);

    }

    spinlock_release(&blockedtasks.spinlock);

}

static void checksignals(struct task *task)
{

    struct list_item *taskitem = &taskdata[task->id].item;

    if (task->signals.kills)
    {

        if (task_transition(task, TASK_STATE_DEAD))
            list_add(&deadtasks, taskitem);

    }

    else if (task->signals.blocks)
    {

        if (task_transition(task, TASK_STATE_BLOCKED))
            list_add(&blockedtasks, taskitem);

    }

    else
    {

        if (task_transition(task, TASK_STATE_ASSIGNED))
            coreassign(taskitem);

    }

    task_resetsignals(&task->signals);

}

static struct task *picknewtask(struct core *core)
{

    struct list_item *taskitem = list_picktail(&core->tasks);

    if (taskitem)
    {

        struct task *task = taskitem->data;

        if (task_transition(task, TASK_STATE_RUNNING))
            return task;

    }

    return 0;

}

static unsigned int setupbinary(struct task *task, unsigned int sp, struct service *service, unsigned int id)
{

    task->node.address = service->map(id);

    if (!task->node.address)
        return 0;

    task->format = binary_findformat(&task->node);

    if (!task->format)
        return 0;

    task->thread.ip = task->format->findentry(&task->node);
    task->thread.sp = sp;

    return task->id;

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

        struct link *link = linkitem->data;

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

        struct link *link = current->data;

        next = current->next;

        if (link->target == target)
        {

            list_remove_unsafe(list, current);
            list_add(&freelinks, current);

        }

    }

    spinlock_release(&list->spinlock);

}

struct task *kernel_schedule(struct core *core)
{

    if (core->task)
        checksignals(core->task);

    unblocktasks();

    return picknewtask(core);

}

struct descriptor *kernel_getdescriptor(struct task *task, unsigned int descriptor)
{

    return &taskdata[task->id].descriptors[(descriptor & (KERNEL_DESCRIPTORS - 1))];

}

void kernel_kill(unsigned int source, unsigned int target)
{

    struct task *task = &taskdata[target].task;

    task_signal(task, TASK_SIGNAL_KILL);

}

unsigned int kernel_pick(unsigned int source, struct message *message, void *data)
{

    struct mailbox *mailbox = &taskdata[source].mailbox;

    return mailbox_pick(mailbox, message, data);

}

unsigned int kernel_place(unsigned int source, unsigned int target, struct message *message, void *data)
{

    struct mailbox *mailbox = &taskdata[target].mailbox;

    message->source = source;

    return mailbox_place(mailbox, message, data);

}

void kernel_notify(struct list *links, unsigned int type, void *buffer, unsigned int count)
{

    struct message message;
    struct list_item *current;

    message_init(&message, type, count);
    spinlock_acquire(&links->spinlock);

    for (current = links->head; current; current = current->next)
    {

        struct link *link = current->data;

        kernel_place(link->source, link->target, &message, buffer);

    }

    spinlock_release(&links->spinlock);

}

struct task *kernel_createtask(void)
{

    struct list_item *taskitem = list_picktail(&deadtasks);

    if (taskitem)
    {

        struct task *task = taskitem->data;
        struct mailbox *mailbox = &taskdata[task->id].mailbox;
        unsigned int i;

        task_reset(task);
        mailbox_reset(mailbox);

        for (i = 0; i < KERNEL_DESCRIPTORS; i++)
            descriptor_reset(kernel_getdescriptor(task, i));

        if (task_transition(task, TASK_STATE_NEW))
            return task;

    }

    return 0;

}

void kernel_setuptask(struct task *task, unsigned int sp)
{

    struct descriptor *prog = kernel_getdescriptor(task, FILE_PP);

    if (setupbinary(task, sp, prog->service, prog->id))
    {

        if (task_transition(task, TASK_STATE_ASSIGNED))
            coreassign(&taskdata[task->id].item);

    }

    else
    {

        if (task_transition(task, TASK_STATE_DEAD))
            list_add(&deadtasks, &taskdata[task->id].item);

    }

}

void kernel_setup(unsigned int mbaddress, unsigned int mbsize)
{

    unsigned int i;

    list_init(&freelinks);
    list_init(&deadtasks);
    list_init(&blockedtasks);

    for (i = 1; i < KERNEL_TASKS; i++)
    {

        struct task *task = &taskdata[i].task;
        struct mailbox *mailbox = &taskdata[i].mailbox;
        struct list_item *item = &taskdata[i].item;
        unsigned int j;

        task_init(task, i);
        task_register(task);
        mailbox_init(mailbox, (void *)(mbaddress + i * mbsize), mbsize);
        mailbox_register(mailbox);
        list_inititem(item, task);
        list_add(&deadtasks, item);

        for (j = 0; j < KERNEL_DESCRIPTORS; j++)
        {

            struct descriptor *descriptor = &taskdata[i].descriptors[j];

            descriptor_init(descriptor);

        }

    }

    for (i = 0; i < KERNEL_LINKS; i++)
    {

        struct link *link = &linkdata[i].link;
        struct list_item *item = &linkdata[i].item;

        link_init(link);
        list_inititem(item, link);
        list_add(&freelinks, item);

    }

}

