#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "mailbox.h"
#include "task.h"
#include "core.h"
#include "link.h"
#include "service.h"
#include "kernel.h"

static struct task tasks[KERNEL_TASKS];
static struct mailbox mailboxes[KERNEL_MAILBOXES];
static struct link links[KERNEL_LINKS];
static struct service_descriptor descriptors[KERNEL_DESCRIPTORS * KERNEL_TASKS];
static struct list freelinks;
static struct list freetasks;
static struct list blockedtasks;
static struct core *(*coreget)(void);
static void (*coreassign)(struct task *task);

static void copydescriptor(struct service_descriptor *descriptor, struct service_descriptor *pdescriptor)
{

    descriptor->protocol = pdescriptor->protocol;
    descriptor->id = pdescriptor->id;

}

static unsigned int setupbinary(struct task *task, unsigned int sp)
{

    struct service_descriptor *init = kernel_getdescriptor(task, FILE_CP);

    if (!init)
        return 0;

    task->node.address = init->protocol->map(init->id);

    if (!task->node.address)
        return 0;

    task->format = binary_findformat(&task->node);

    if (!task->format)
        return 0;

    task->thread.ip = task->format->findentry(&task->node);
    task->thread.sp = sp;

    return task->id;

}

unsigned int kernel_walk(struct service_descriptor *descriptor, char *path, unsigned int length)
{

    unsigned int colon = buffer_findbyte(path, length, ':');
    unsigned int offset = 0;

    if (colon < length)
    {

        struct service_protocol *protocol = service_findprotocol(colon, path);

        if (protocol)
        {

            descriptor->protocol = protocol;
            descriptor->id = protocol->root();
            offset += colon + 1;

        }

    }

    while (offset < length)
    {

        char *cp = path + offset;
        unsigned int cl = length - offset;

        cl = buffer_findbyte(cp, cl, '/');

        if (cl == 2 && cp[0] == '.' && cp[1] == '.')
            descriptor->id = descriptor->protocol->parent(descriptor->id);
        else
            descriptor->id = descriptor->protocol->child(descriptor->id, cp, cl);

        if (!descriptor->id)
            return 0;

        offset += cl + 1;

    }

    return descriptor->id;

}

struct core *kernel_getcore(void)
{

    return coreget();

}

void kernel_setcallback(struct core *(*get)(void), void (*assign)(struct task *task))
{

    coreget = get;
    coreassign = assign;

}

struct link *kernel_picklink(unsigned int source)
{

    struct list_item *current = list_picktail(&freelinks);

    if (current)
    {

        struct link *link = current->data;

        link->source = source;

        return link;

    }

    return 0;

}

void kernel_freelink(struct link *link)
{

    link->source = 0;

    list_add(&freelinks, &link->item);

}

void kernel_schedule(struct core *core)
{

    struct list_item *current;
    struct list_item *next;

    if (core->task)
    {

        switch (core->task->state)
        {

        case TASK_STATE_NORMAL:
            list_add(&core->tasks, &core->task->item);

            break;

        case TASK_STATE_KILLED:
            list_add(&freetasks, &core->task->item);

            break;

        case TASK_STATE_BLOCKED:
            list_add(&blockedtasks, &core->task->item);

            break;

        }

        task_setstate(core->task, TASK_STATE_NORMAL);

        core->task = 0;

    }

    spinlock_acquire(&blockedtasks.spinlock);

    for (current = blockedtasks.head; current; current = next)
    {

        struct task *task = current->data;
        struct mailbox *mailbox = &mailboxes[task->id];

        next = current->next;

        spinlock_acquire(&mailbox->spinlock);

        if (ring_count(&mailbox->ring))
        {

            list_remove_nolock(&blockedtasks, current);
            coreassign(task);

        }

        spinlock_release(&mailbox->spinlock);

    }

    spinlock_release(&blockedtasks.spinlock);
    core_sorttasks(core);

    current = list_picktail(&core->tasks);

    core->task = (current) ? current->data : 0;

}

struct service_descriptor *kernel_getdescriptor(struct task *task, unsigned int descriptor)
{

    return &descriptors[task->id * KERNEL_DESCRIPTORS + (descriptor & (KERNEL_DESCRIPTORS - 1))];

}

void kernel_kill(unsigned int source, unsigned int target)
{

    struct task *task = &tasks[target];

    task_setstate(task, TASK_STATE_KILLED);

}

unsigned int kernel_pick(unsigned int source, struct message_header *header, void *data)
{

    struct task *task = &tasks[source];
    struct mailbox *mailbox = &mailboxes[source];
    unsigned int count = mailbox_pick(mailbox, header, data);

    if (!count)
        task_setstate(task, TASK_STATE_BLOCKED);

    return count;

}

unsigned int kernel_place(unsigned int source, unsigned int target, struct message_header *header, void *data)
{

    struct task *task = &tasks[target];
    struct mailbox *mailbox = &mailboxes[target];

    header->source = source;

    spinlock_acquire(&task->spinlock);

    task->kicked++;

    spinlock_release(&task->spinlock);

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

unsigned int kernel_loadtask(struct task *parent, unsigned int sp)
{

    struct list_item *current = list_picktail(&freetasks);

    if (current)
    {

        struct task *task = current->data;
        struct mailbox *mailbox = &mailboxes[task->id];

        mailbox_reset(mailbox);
        buffer_clear(kernel_getdescriptor(task, FILE_PP), sizeof (struct service_descriptor) * KERNEL_DESCRIPTORS);

        if (parent)
        {

            copydescriptor(kernel_getdescriptor(task, FILE_CP), kernel_getdescriptor(parent, FILE_CP));
            copydescriptor(kernel_getdescriptor(task, FILE_CR), kernel_getdescriptor(parent, FILE_CR));
            copydescriptor(kernel_getdescriptor(task, FILE_CW), kernel_getdescriptor(parent, FILE_CW));

        }

        else
        {

            struct service_descriptor *prog = kernel_getdescriptor(task, FILE_CP);
            struct service_descriptor *root = kernel_getdescriptor(task, FILE_CR);
            struct service_descriptor *work = kernel_getdescriptor(task, FILE_CW);

            root->protocol = service_findprotocol(6, "initrd");
            root->id = root->protocol->root();

            copydescriptor(work, root);
            copydescriptor(prog, root);

            prog->id = prog->protocol->child(prog->id, "bin", 3);
            prog->id = prog->protocol->child(prog->id, "init", 4);

        }

        copydescriptor(kernel_getdescriptor(task, FILE_PP), kernel_getdescriptor(task, FILE_CP));
        copydescriptor(kernel_getdescriptor(task, FILE_PR), kernel_getdescriptor(task, FILE_CR));
        copydescriptor(kernel_getdescriptor(task, FILE_PW), kernel_getdescriptor(task, FILE_CW));

        if (setupbinary(task, sp))
        {

            coreassign(task);

            return task->id;

        }

        else
        {

            list_add(&freetasks, &task->item);

            return 0;

        }

    }

    return 0;

}

void kernel_setup(unsigned int mbaddress, unsigned int mbsize)
{

    unsigned int i;

    list_init(&freelinks);
    list_init(&freetasks);
    list_init(&blockedtasks);

    for (i = 1; i < KERNEL_TASKS; i++)
    {

        struct task *task = &tasks[i];

        task_init(task, i);
        task_register(task);
        list_add(&freetasks, &task->item);

    }

    for (i = 0; i < KERNEL_MAILBOXES; i++)
    {

        struct mailbox *mailbox = &mailboxes[i];

        mailbox_init(mailbox, (void *)(mbaddress + i * mbsize), mbsize);
        mailbox_register(mailbox);

    }

    for (i = 0; i < KERNEL_LINKS; i++)
    {

        struct link *link = &links[i];

        link_init(link);
        list_add(&freelinks, &link->item);

    }

}

