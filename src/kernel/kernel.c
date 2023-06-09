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

static struct taskrow {struct task task; struct mailbox mailbox; struct descriptor descriptors[KERNEL_DESCRIPTORS]; struct list_item item;} taskrows[KERNEL_TASKS];
static struct linkrow {struct link link; struct list_item item;} linkrows[KERNEL_LINKS];
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
        struct mailbox *mailbox = &taskrows[task->id].mailbox;

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

    struct taskrow *taskrow = &taskrows[task->id];

    if (task->signals.kills)
    {

        if (task_transition(task, TASK_STATE_DEAD))
            list_add(&deadtasks, &taskrow->item);

    }

    else if (task->signals.blocks)
    {

        if (task_transition(task, TASK_STATE_BLOCKED))
            list_add(&blockedtasks, &taskrow->item);

    }

    else
    {

        if (task_transition(task, TASK_STATE_ASSIGNED))
            coreassign(&taskrow->item);

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

    if (task->node.address)
    {

        struct binary_format *format = binary_findformat(&task->node);

        if (format)
        {

            task->thread.ip = format->findentry(&task->node);
            task->thread.sp = sp;

            return task->id;

        }

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

unsigned int kernel_codebase(unsigned int task, unsigned int address)
{

    struct taskrow *taskrow = &taskrows[task];
    struct binary_format *format = binary_findformat(&taskrow->task.node);

    return format->findbase(&taskrow->task.node, address);

}

unsigned int kernel_loadprogram(unsigned int task)
{

    struct taskrow *taskrow = &taskrows[task];
    struct binary_format *format = binary_findformat(&taskrow->task.node);

    return format->copyprogram(&taskrow->task.node);

}

void kernel_signal(unsigned int task, unsigned int signal)
{

    struct taskrow *taskrow = &taskrows[task];

    task_signal(&taskrow->task, signal);

}

struct descriptor *kernel_getdescriptor(unsigned int task, unsigned int descriptor)
{

    struct taskrow *taskrow = &taskrows[task];

    return &taskrow->descriptors[(descriptor & (KERNEL_DESCRIPTORS - 1))];

}

void kernel_setdescriptor(unsigned int task, unsigned int descriptor, struct service *service, unsigned int id)
{

    struct descriptor *desc = kernel_getdescriptor(task, descriptor);

    desc->service = service;
    desc->id = id;

}

void kernel_copydescriptor(unsigned int task, unsigned int descriptor, unsigned int ptask, unsigned int pdescriptor)
{

    struct descriptor *desc = kernel_getdescriptor(task, descriptor);
    struct descriptor *pdesc = kernel_getdescriptor(ptask, pdescriptor);

    desc->service = pdesc->service;
    desc->id = pdesc->id;

}

void kernel_kill(unsigned int source, unsigned int target)
{

    struct taskrow *taskrow = &taskrows[target];

    task_signal(&taskrow->task, TASK_SIGNAL_KILL);

}

unsigned int kernel_pick(unsigned int source, struct message *message, void *data)
{

    struct taskrow *taskrow = &taskrows[source];

    return mailbox_pick(&taskrow->mailbox, message, data);

}

unsigned int kernel_place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct taskrow *taskrow = &taskrows[target];
    struct message message;

    message_init(&message, event, source, count);

    return mailbox_place(&taskrow->mailbox, &message, data);

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

        struct task *task = taskitem->data;
        struct mailbox *mailbox = &taskrows[task->id].mailbox;
        unsigned int i;

        task_reset(task);
        mailbox_reset(mailbox);

        for (i = 0; i < KERNEL_DESCRIPTORS; i++)
            descriptor_reset(kernel_getdescriptor(task->id, i));

        if (task_transition(task, TASK_STATE_NEW))
            return task->id;

    }

    return 0;

}

void kernel_setuptask(unsigned int task, unsigned int sp, unsigned int descriptor)
{

    struct descriptor *pdescriptor = kernel_getdescriptor(task, descriptor);
    struct taskrow *taskrow = &taskrows[task];

    if (setupbinary(&taskrow->task, sp, pdescriptor->service, pdescriptor->id))
    {

        if (task_transition(&taskrow->task, TASK_STATE_ASSIGNED))
            coreassign(&taskrow->item);

    }

    else
    {

        if (task_transition(&taskrow->task, TASK_STATE_DEAD))
            list_add(&deadtasks, &taskrow->item);

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

        struct taskrow *taskrow = &taskrows[i];
        unsigned int j;

        task_init(&taskrow->task, i);
        task_register(&taskrow->task);
        mailbox_init(&taskrow->mailbox, (void *)(mbaddress + i * mbsize), mbsize);
        mailbox_register(&taskrow->mailbox);
        list_inititem(&taskrow->item, &taskrow->task);
        list_add(&deadtasks, &taskrow->item);

        for (j = 0; j < KERNEL_DESCRIPTORS; j++)
            descriptor_init(&taskrow->descriptors[j]);

    }

    for (i = 0; i < KERNEL_LINKS; i++)
    {

        struct linkrow *linkrow = &linkrows[i];

        link_init(&linkrow->link);
        list_inititem(&linkrow->item, &linkrow->link);
        list_add(&freelinks, &linkrow->item);

    }

}

