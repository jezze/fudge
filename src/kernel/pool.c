#include <fudge.h>
#include "resource.h"
#include "service.h"
#include "mailbox.h"
#include "core.h"
#include "task.h"
#include "mmap.h"
#include "pool.h"

static struct corerow {struct list_item item; struct core core;} corerows[POOL_CORES];
static struct taskrow {struct list_item item; struct task task;} taskrows[POOL_TASKS];
static struct mailboxrow {struct list_item item; struct mailbox mailbox;} mailboxrows[POOL_MAILBOXES];
static struct noderow {struct list_item item; struct resource *resource; unsigned int generation;} noderows[POOL_NODES];
static struct list freetasks;
static struct list freemailboxes;
static struct list freenodes;
static struct spinlock attachlock;

static struct corerow *getcorerow(unsigned int icore)
{

    return (icore < POOL_CORES) ? &corerows[icore] : 0;

}

static struct mailboxrow *getmailboxrow(unsigned int imailbox)
{

    return (imailbox && imailbox < POOL_MAILBOXES) ? &mailboxrows[imailbox] : 0;

}

static struct noderow *getnoderow(unsigned int inode)
{

    unsigned int index = inode % POOL_NODES;

    return (index && noderows[index].generation == inode / POOL_NODES) ? &noderows[index] : 0;

}

static struct taskrow *gettaskrow(unsigned int itask)
{

    return (itask && itask < POOL_TASKS) ? &taskrows[itask] : 0;

}

static unsigned int encodecorerow(struct corerow *corerow)
{

    return corerow - corerows;

}

static unsigned int encodemailboxrow(struct mailboxrow *mailboxrow)
{

    return mailboxrow - mailboxrows;

}

static unsigned int encodenoderow(struct noderow *noderow)
{

    return (noderow - noderows) + POOL_NODES * noderow->generation;

}

static unsigned int encodetaskrow(struct taskrow *taskrow)
{

    return taskrow - taskrows;

}

struct core *pool_getcore(unsigned int icore)
{

    struct corerow *corerow = getcorerow(icore);

    return corerow ? &corerow->core : 0;

}

struct mailbox *pool_getmailbox(unsigned int imailbox)
{

    struct mailboxrow *mailboxrow = getmailboxrow(imailbox);

    return mailboxrow ? &mailboxrow->mailbox : 0;

}

struct resource *pool_getnoderesource(unsigned int inode)
{

    struct noderow *noderow = getnoderow(inode);

    return noderow ? noderow->resource : 0;

}

struct task *pool_gettask(unsigned int itask)
{

    struct taskrow *taskrow = gettaskrow(itask);

    return taskrow ? &taskrow->task : 0;

}

unsigned int pool_getinodefromitem(struct list_item *item)
{

    return encodenoderow(item->data);

}

unsigned int pool_pickcorefrom(struct list *list)
{

    struct list_item *item = list_pickhead(list);

    return (item) ? encodecorerow(item->data) : POOL_CORES;

}

unsigned int pool_pickmailbox(void)
{

    struct list_item *item = list_pickhead(&freemailboxes);

    return (item) ? encodemailboxrow(item->data) : 0;

}

unsigned int pool_picknode(struct resource *resource)
{

    struct list_item *item = list_pickhead(&freenodes);

    if (item)
    {

        struct noderow *noderow = item->data;

        noderow->resource = resource;

        return encodenoderow(item->data);

    }

    return 0;

}

unsigned int pool_attachnode(struct resource *resource)
{

    unsigned int inode;

    spinlock_acquire(&attachlock);

    if (!resource->inode)
        resource->inode = pool_picknode(resource);

    inode = resource->inode;

    spinlock_release(&attachlock);

    return inode;

}

void pool_detachnode(struct resource *resource)
{

    struct list_item *item;
    unsigned int inode;

    spinlock_acquire(&attachlock);

    inode = resource->inode;
    resource->inode = 0;

    spinlock_release(&attachlock);

    while ((item = list_pickhead(&resource->links)))
        pool_unpicknode(pool_getinodefromitem(item));

    pool_unpicknode(inode);

}

void pool_setroot(struct resource *resource)
{

    noderows[MESSAGE_ROOT].resource = resource;

}

unsigned int pool_picktask(void)
{

    return pool_picktaskfrom(&freetasks);

}

unsigned int pool_picktaskfrom(struct list *list)
{

    struct list_item *item = list_pickhead(list);

    return (item) ? encodetaskrow(item->data) : 0;

}

void pool_unpickmailbox(unsigned int imailbox)
{

    struct mailboxrow *mailboxrow = getmailboxrow(imailbox);

    if (mailboxrow)
        list_add(&freemailboxes, &mailboxrow->item);

}

void pool_unpicknode(unsigned int inode)
{

    struct noderow *noderow = getnoderow(inode);

    if (noderow)
    {

        noderow->generation++;

        list_add(&freenodes, &noderow->item);

    }

}

unsigned int pool_renewnode(unsigned int inode)
{

    struct noderow *noderow = getnoderow(inode);

    if (noderow)
    {

        noderow->generation++;

        return encodenoderow(noderow);

    }

    return 0;

}

void pool_unpicktask(unsigned int itask)
{

    struct taskrow *taskrow = gettaskrow(itask);

    if (taskrow)
        list_add(&freetasks, &taskrow->item);

}

void pool_placecore(unsigned int icore, struct list *list)
{

    struct corerow *corerow = getcorerow(icore);

    if (corerow)
        list_add(list, &corerow->item);

}

void pool_placenode(unsigned int inode, struct list *list)
{

    struct noderow *noderow = getnoderow(inode);

    if (noderow)
        list_add(list, &noderow->item);

}

void pool_placetask(unsigned int itask, struct list *list)
{

    struct taskrow *taskrow = gettaskrow(itask);

    if (taskrow)
        list_add(list, &taskrow->item);

}

void pool_setup(unsigned long mbaddress)
{

    unsigned int i;

    list_init(&freetasks);
    list_init(&freemailboxes);
    list_init(&freenodes);
    spinlock_init(&attachlock);

    for (i = 0; i < POOL_CORES; i++)
    {

        struct corerow *corerow = &corerows[i];

        core_init(&corerow->core);
        list_inititem(&corerow->item, corerow);

    }

    /* MESSAGE_ROOT is never free and never renewed, so its inode stays the same */
    for (i = MESSAGE_ROOT + 1; i < POOL_NODES; i++)
    {

        struct noderow *noderow = &noderows[i];

        list_inititem(&noderow->item, noderow);
        list_add(&freenodes, &noderow->item);

    }

    for (i = 1; i < POOL_MAILBOXES; i++)
    {

        struct mailboxrow *mailboxrow = &mailboxrows[i];

        mailbox_init(&mailboxrow->mailbox, mbaddress + i * MESSAGE_CAPACITY);
        list_inititem(&mailboxrow->item, mailboxrow);
        list_add(&freemailboxes, &mailboxrow->item);

    }

    for (i = 1; i < POOL_TASKS; i++)
    {

        struct taskrow *taskrow = &taskrows[i];

        task_init(&taskrow->task);
        list_inititem(&taskrow->item, taskrow);
        list_add(&freetasks, &taskrow->item);

    }

}

