#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "mailbox.h"
#include "task.h"
#include "core.h"
#include "service.h"
#include "kernel.h"

static struct task tasks[KERNEL_TASKS];
static struct mailbox mailboxes[KERNEL_MAILBOXES];
static struct service_descriptor descriptors[KERNEL_DESCRIPTORS * KERNEL_TASKS];
static struct service_mount mounts[KERNEL_MOUNTS];
static struct list usedtasks;
static struct list freetasks;
static struct list blockedtasks;
static struct list usedmounts;
static struct list freemounts;
static struct core *(*coreget)(void);
static void (*coreassign)(struct task *task);

static unsigned int walkmount(struct service_descriptor *descriptor, struct service_mountpoint *from, struct service_mountpoint *to)
{

    if (descriptor->protocol == from->protocol && descriptor->id == from->id)
    {

        descriptor->protocol = to->protocol;
        descriptor->id = to->id;

        return 1;

    }

    return 0;

}

static void walkmountparent(struct service_descriptor *descriptor)
{

    struct list_item *current;

    spinlock_acquire(&usedmounts.spinlock);

    for (current = usedmounts.head; current; current = current->next)
    {

        struct service_mount *mount = current->data;

        if (walkmount(descriptor, &mount->child, &mount->parent))
            break;

    }

    spinlock_release(&usedmounts.spinlock);

}

static void walkmountchild(struct service_descriptor *descriptor)
{

    struct list_item *current;

    spinlock_acquire(&usedmounts.spinlock);

    for (current = usedmounts.head; current; current = current->next)
    {

        struct service_mount *mount = current->data;

        if (walkmount(descriptor, &mount->parent, &mount->child))
            break;

    }

    spinlock_release(&usedmounts.spinlock);

}

unsigned int kernel_walk(struct service_descriptor *descriptor, char *path, unsigned int length)
{

    unsigned int offset = 0;

    while (offset < length)
    {

        char *cp = path + offset;
        unsigned int cl = length - offset;

        cl = memory_findbyte(cp, cl, '/');

        if (cl == 2 && cp[0] == '.' && cp[1] == '.')
        {

            walkmountparent(descriptor);

            descriptor->id = descriptor->protocol->parent(descriptor->id);

        }

        else
        {

            descriptor->id = descriptor->protocol->child(descriptor->id, cp, cl);

            walkmountchild(descriptor);

        }

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

struct task *kernel_picktask(void)
{

    struct list_item *current = list_picktail(&freetasks);

    return (current) ? current->data : 0;

}

struct service_mount *kernel_pickmount(void)
{

    struct list_item *current = list_picktail(&freemounts);

    return (current) ? current->data : 0;

}

void kernel_usetask(struct task *task)
{

    list_add(&usedtasks, &task->item);

}

void kernel_usemount(struct service_mount *mount)
{

    list_add(&usedmounts, &mount->item);

}

void kernel_freetask(struct task *task)
{

    list_add(&freetasks, &task->item);

}

void kernel_freemount(struct service_mount *mount)
{

    list_add(&freemounts, &mount->item);

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
            coreassign(task);

        }

        spinlock_release(&mailbox->spinlock);

    }

    spinlock_release(&blockedtasks.spinlock);

    while ((current = list_pickhead(&usedtasks)))
    {

        struct task *task = current->data;

        coreassign(task);

    }

}

struct service_descriptor *kernel_getdescriptor(struct task *task, unsigned int descriptor)
{

    return &descriptors[task->id * KERNEL_DESCRIPTORS + (descriptor & (KERNEL_DESCRIPTORS - 1))];

}

static void copydescriptor(struct service_descriptor *tdescriptor, struct service_descriptor *sdescriptor, struct task *task)
{

    tdescriptor->protocol = (sdescriptor) ? sdescriptor->protocol : 0;
    tdescriptor->id = (sdescriptor) ? sdescriptor->id : 0;

}

void kernel_copydescriptors(struct task *source, struct task *target)
{

    unsigned int i;

    for (i = 0x00; i < 0x04; i++)
    {

        copydescriptor(kernel_getdescriptor(target, i + FILE_PP), kernel_getdescriptor(source, i + FILE_CP), target);
        copydescriptor(kernel_getdescriptor(target, i + FILE_CP), kernel_getdescriptor(source, i + FILE_CP), target);

    }

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

    message_initheader(&header, type, count);
    spinlock_acquire(&links->spinlock);

    for (current = links->head; current; current = current->next)
    {

        struct service_link *link = current->data;

        kernel_place(link->id, &header, buffer);

    }

    spinlock_release(&links->spinlock);

}

unsigned int kernel_setupbinary(struct task *task, unsigned int descriptor, unsigned int sp)
{

    struct service_descriptor *init = kernel_getdescriptor(task, descriptor);

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

    root->protocol = service_findprotocol(1000);
    root->id = root->protocol->root();
    work->protocol = root->protocol;
    work->id = work->protocol->root();
    init->protocol = root->protocol;
    init->id = init->protocol->root();
    init->id = init->protocol->child(init->id, "bin", 3);
    init->id = init->protocol->child(init->id, "init", 4);

}

void kernel_setup(unsigned int mbaddress, unsigned int mbsize)
{

    unsigned int i;

    list_init(&usedtasks);
    list_init(&freetasks);
    list_init(&blockedtasks);
    list_init(&usedmounts);
    list_init(&freemounts);

    for (i = 1; i < KERNEL_TASKS; i++)
    {

        struct task *task = &tasks[i];
        unsigned int j;

        task_init(task, i);
        task_register(task);
        kernel_freetask(task);

        for (j = 0; j < KERNEL_DESCRIPTORS; j++)
        {

            struct service_descriptor *descriptor = &descriptors[i * KERNEL_DESCRIPTORS + j];

            service_initdescriptor(descriptor, task->id);

        }

    }

    for (i = 0; i < KERNEL_MAILBOXES; i++)
    {

        struct mailbox *mailbox = &mailboxes[i];

        mailbox_init(mailbox, (char *)(mbaddress + i * mbsize), mbsize);
        mailbox_register(mailbox);

    }

    for (i = 0; i < KERNEL_MOUNTS; i++)
    {

        struct service_mount *mount = &mounts[i];

        service_initmount(mount);
        kernel_freemount(mount);

    }

}

