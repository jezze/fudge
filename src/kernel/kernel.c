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
static struct list readytasks;
static struct list blockedtasks;
static struct list killedtasks;
static struct core *(*coreget)(void);
static void (*coreassign)(struct task *task);

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

        offset += cl + 1;

    }

    return offset >= length;

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

struct task *kernel_picktask(void)
{

    struct list_item *current = list_picktail(&freetasks);

    return (current) ? current->data : 0;

}

void kernel_freetask(unsigned int id)
{

    struct task *task = &tasks[id];

    list_add(&freetasks, &task->item);

}

void kernel_readytask(unsigned int id)
{

    struct task *task = &tasks[id];

    list_add(&readytasks, &task->item);

}

void kernel_killtask(unsigned int id)
{

    struct task *task = &tasks[id];

    list_add(&killedtasks, &task->item);

}

void kernel_assign(void)
{

    struct list_item *current;
    struct list_item *next;

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
            kernel_readytask(task->id);

        }

        spinlock_release(&mailbox->spinlock);

    }

    spinlock_release(&blockedtasks.spinlock);

    while ((current = list_pickhead(&readytasks)))
    {

        struct task *task = current->data;

        coreassign(task);

    }

}

struct service_descriptor *kernel_getdescriptor(struct task *task, unsigned int descriptor)
{

    return &descriptors[task->id * KERNEL_DESCRIPTORS + (descriptor & (KERNEL_DESCRIPTORS - 1))];

}

static void copydescriptor(struct service_descriptor *tdescriptor, struct service_descriptor *sdescriptor)
{

    tdescriptor->protocol = sdescriptor->protocol;
    tdescriptor->id = sdescriptor->id;

}

void kernel_copydescriptors(struct task *source, struct task *target)
{

    copydescriptor(kernel_getdescriptor(target, FILE_PP + 0), kernel_getdescriptor(source, FILE_CP + 0));
    copydescriptor(kernel_getdescriptor(target, FILE_CP + 0), kernel_getdescriptor(source, FILE_CP + 0));
    copydescriptor(kernel_getdescriptor(target, FILE_PP + 1), kernel_getdescriptor(source, FILE_CP + 1));
    copydescriptor(kernel_getdescriptor(target, FILE_CP + 1), kernel_getdescriptor(source, FILE_CP + 1));
    copydescriptor(kernel_getdescriptor(target, FILE_PP + 2), kernel_getdescriptor(source, FILE_CP + 2));
    copydescriptor(kernel_getdescriptor(target, FILE_CP + 2), kernel_getdescriptor(source, FILE_CP + 2));

}

void kernel_reset(unsigned int id)
{

    mailbox_reset(&mailboxes[id]);

}

unsigned int kernel_pick(unsigned int id, struct message_header *header, void *data)
{

    unsigned int count = mailbox_pick(&mailboxes[id], header, data);

    if (!count)
        list_add(&blockedtasks, &tasks[id].item);

    return count;

}

unsigned int kernel_place(unsigned int id, struct message_header *header, void *data)
{

    return mailbox_place(&mailboxes[id], header, data);

}

void kernel_notify(struct list *links, unsigned int type, void *buffer, unsigned int count)
{

    struct message_header header;
    struct list_item *current;

    message_initheader(&header, type, 0, count);
    spinlock_acquire(&links->spinlock);

    for (current = links->head; current; current = current->next)
    {

        struct link *link = current->data;

        kernel_place(link->source, &header, buffer);

    }

    spinlock_release(&links->spinlock);

}

unsigned int kernel_setupbinary(struct task *task, unsigned int sp)
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

    return 1;

}

void kernel_setupinit(struct task *task)
{

    struct service_descriptor *init = kernel_getdescriptor(task, FILE_CP);
    struct service_descriptor *root = kernel_getdescriptor(task, FILE_CR);
    struct service_descriptor *work = kernel_getdescriptor(task, FILE_CW);

    root->protocol = service_findprotocol(6, "initrd");
    root->id = root->protocol->root();

    copydescriptor(work, root);
    copydescriptor(init, root);

    init->id = init->protocol->child(init->id, "bin", 3);
    init->id = init->protocol->child(init->id, "init", 4);

}

void kernel_setup(unsigned int mbaddress, unsigned int mbsize)
{

    unsigned int i;

    list_init(&freelinks);
    list_init(&freetasks);
    list_init(&readytasks);
    list_init(&blockedtasks);
    list_init(&killedtasks);

    for (i = 1; i < KERNEL_TASKS; i++)
    {

        struct task *task = &tasks[i];

        task_init(task, i);
        task_register(task);
        kernel_freetask(task->id);

    }

    for (i = 0; i < KERNEL_MAILBOXES; i++)
    {

        struct mailbox *mailbox = &mailboxes[i];

        mailbox_init(mailbox, (char *)(mbaddress + i * mbsize), mbsize);
        mailbox_register(mailbox);

    }

    for (i = 0; i < KERNEL_LINKS; i++)
    {

        struct link *link = &links[i];

        link_init(link);
        list_add(&freelinks, &link->item);

    }

}

