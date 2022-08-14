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
    struct list_item item;

};

struct linkdata
{

    struct link link;
    struct list_item item;

};

static struct taskdata taskdata[KERNEL_TASKS];
static struct linkdata linkdata[KERNEL_LINKS];
static struct descriptor descriptors[KERNEL_DESCRIPTORS * KERNEL_TASKS];
static struct mailbox mailboxes[KERNEL_MAILBOXES];
static struct list freelinks;
static struct list killedtasks;
static struct list blockedtasks;
static struct core *(*coreget)(void);
static void (*coreassign)(struct list_item *item);

static unsigned int setupbinary(struct task *task, unsigned int sp)
{

    struct descriptor *prog = kernel_getdescriptor(task, FILE_CP);

    if (!prog)
        return 0;

    task->node.address = prog->service->map(prog->id);

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

void kernel_addlink(unsigned int source, struct list *list)
{

    struct list_item *linkitem = list_picktail(&freelinks);

    if (linkitem)
    {

        struct link *link = linkitem->data;

        link->source = source;

        list_add(list, linkitem);

    }

}

void kernel_removelink(unsigned int source, struct list *list)
{

    struct list_item *current;
    struct list_item *next;

    spinlock_acquire(&list->spinlock);

    for (current = list->head; current; current = next)
    {

        struct link *link = current->data;

        next = current->next;

        if (link->source == source)
        {

            list_remove_unsafe(list, current);
            list_add_unsafe(&freelinks, current);

        }

    }

    spinlock_release(&list->spinlock);

}

struct task *kernel_schedule(struct core *core, struct task *coretask)
{

    struct list_item *taskitem;
    struct list_item *next;

    if (coretask)
    {

        if (coretask->signals.kills)
        {

            task_unsignal(coretask, TASK_SIGNAL_KILL);
            task_transition(coretask, TASK_STATE_KILLED);
            list_add(&killedtasks, &taskdata[coretask->id].item);

        }

        else if (coretask->signals.blocks)
        {

            task_unsignal(coretask, TASK_SIGNAL_BLOCK);
            task_transition(coretask, TASK_STATE_BLOCKED);
            list_add(&blockedtasks, &taskdata[coretask->id].item);

        }

        else
        {

            task_transition(coretask, TASK_STATE_ASSIGNED);
            coreassign(&taskdata[coretask->id].item);

        }

    }

    spinlock_acquire(&blockedtasks.spinlock);

    for (taskitem = blockedtasks.head; taskitem; taskitem = next)
    {

        struct task *task = taskitem->data;
        struct mailbox *mailbox = &mailboxes[task->id];

        next = taskitem->next;

        spinlock_acquire(&mailbox->spinlock);

        if (ring_count(&mailbox->ring))
        {

            list_remove_unsafe(&blockedtasks, taskitem);
            task_unsignal(task, TASK_SIGNAL_UNBLOCK);
            task_transition(task, TASK_STATE_ASSIGNED);
            coreassign(&taskdata[task->id].item);

        }

        spinlock_release(&mailbox->spinlock);

    }

    spinlock_release(&blockedtasks.spinlock);

    taskitem = list_picktail(&core->tasks);

    if (taskitem)
    {

        struct task *task = taskitem->data;

        task_transition(task, TASK_STATE_RUNNING);

        return task;

    }

    return 0;

}

struct descriptor *kernel_getdescriptor(struct task *task, unsigned int descriptor)
{

    return &descriptors[task->id * KERNEL_DESCRIPTORS + (descriptor & (KERNEL_DESCRIPTORS - 1))];

}

void kernel_kill(unsigned int source, unsigned int target)
{

    struct task *task = &taskdata[target].task;

    task_signal(task, TASK_SIGNAL_KILL);

}

unsigned int kernel_pick(unsigned int source, struct message_header *header, void *data)
{

    struct task *task = &taskdata[source].task;
    struct mailbox *mailbox = &mailboxes[source];
    unsigned int count = mailbox_pick(mailbox, header, data);

    if (!count)
        task_signal(task, TASK_SIGNAL_BLOCK);

    return count;

}

unsigned int kernel_place(unsigned int source, unsigned int target, struct message_header *header, void *data)
{

    struct task *task = &taskdata[target].task;
    struct mailbox *mailbox = &mailboxes[target];

    header->source = source;

    task_signal(task, TASK_SIGNAL_UNBLOCK);

    return mailbox_place(mailbox, header, data);

}

void kernel_notify(struct list *links, unsigned int type, void *buffer, unsigned int count)
{

    struct message_header header;
    struct list_item *current;

    message_initheader(&header, type, count);
    spinlock_acquire(&links->spinlock);

    for (current = links->head; current; current = current->next)
    {

        struct link *link = current->data;

        kernel_place(0, link->source, &header, buffer);

    }

    spinlock_release(&links->spinlock);

}

struct task *kernel_loadtask(struct task *parent, unsigned int sp)
{

    struct list_item *taskitem = list_picktail(&killedtasks);

    if (taskitem)
    {

        struct task *task = taskitem->data;
        struct mailbox *mailbox = &mailboxes[task->id];
        unsigned int i;

        task_reset(task);
        mailbox_reset(mailbox);

        for (i = 0; i < KERNEL_DESCRIPTORS; i++)
            descriptor_reset(kernel_getdescriptor(task, i));

        if (parent)
        {

            descriptor_copy(kernel_getdescriptor(task, FILE_CP), kernel_getdescriptor(parent, FILE_CP));
            descriptor_copy(kernel_getdescriptor(task, FILE_CW), kernel_getdescriptor(parent, FILE_CW));

        }

        else
        {

            struct descriptor *prog = kernel_getdescriptor(task, FILE_CP);
            struct descriptor *work = kernel_getdescriptor(task, FILE_CW);

            work->service = service_find(6, "initrd");
            work->id = work->service->root();

            descriptor_copy(prog, work);

            prog->id = prog->service->child(prog->id, "bin", 3);
            prog->id = prog->service->child(prog->id, "init", 4);

        }

        descriptor_copy(kernel_getdescriptor(task, FILE_PP), kernel_getdescriptor(task, FILE_CP));
        descriptor_copy(kernel_getdescriptor(task, FILE_PW), kernel_getdescriptor(task, FILE_CW));

        if (setupbinary(task, sp))
        {

            task_transition(task, TASK_STATE_ASSIGNED);
            coreassign(&taskdata[task->id].item);

            return task;

        }

        else
        {

            task_transition(task, TASK_STATE_KILLED);
            list_add(&killedtasks, &taskdata[task->id].item);

            return 0;

        }

    }

    return 0;

}

void kernel_setup(unsigned int mbaddress, unsigned int mbsize)
{

    unsigned int i;

    list_init(&freelinks);
    list_init(&killedtasks);
    list_init(&blockedtasks);

    for (i = 1; i < KERNEL_TASKS; i++)
    {

        struct task *task = &taskdata[i].task;
        struct list_item *item = &taskdata[i].item;

        task_init(task, i);
        task_register(task);
        list_inititem(item, task);
        list_add(&killedtasks, item);

    }

    for (i = 0; i < KERNEL_DESCRIPTORS * KERNEL_TASKS; i++)
    {

        struct descriptor *descriptor = &descriptors[i];

        descriptor_init(descriptor);

    }

    for (i = 0; i < KERNEL_MAILBOXES; i++)
    {

        struct mailbox *mailbox = &mailboxes[i];

        mailbox_init(mailbox, (void *)(mbaddress + i * mbsize), mbsize);
        mailbox_register(mailbox);

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

