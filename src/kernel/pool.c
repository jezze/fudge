#include <fudge.h>
#include "resource.h"
#include "mailbox.h"
#include "core.h"
#include "task.h"
#include "node.h"
#include "pool.h"

static struct corerow {struct list_item item; struct core core;} corerows[POOL_CORES];
static struct taskrow {struct list_item item; struct task task;} taskrows[POOL_TASKS];
static struct mailboxrow {struct list_item item; struct mailbox mailbox;} mailboxrows[POOL_MAILBOXES];
static struct noderow {struct list_item item; struct node node;} noderows[POOL_NODES];
static struct list freetasks;
static struct list freemailboxes;
static struct list freenodes;
static struct list usednodes;

static struct corerow *getcorerow(unsigned int icore)
{

    return (icore < POOL_CORES) ? &corerows[icore] : 0;

}

static struct noderow *getnoderow(unsigned int inode)
{

    return (inode && inode < POOL_NODES) ? &noderows[inode] : 0;

}

static struct taskrow *gettaskrow(unsigned int itask)
{

    return (itask && itask < POOL_TASKS) ? &taskrows[itask] : 0;

}

static struct mailboxrow *getmailboxrow(unsigned int imailbox)
{

    return (imailbox && imailbox < POOL_MAILBOXES) ? &mailboxrows[imailbox] : 0;

}

static unsigned int encodenoderow(struct noderow *noderow)
{

    return ((unsigned long)noderow - (unsigned long)noderows) / sizeof (struct noderow);

}

static unsigned int encodetaskrow(struct taskrow *taskrow)
{

    return ((unsigned long)taskrow - (unsigned long)taskrows) / sizeof (struct taskrow);

}

static unsigned int encodemailboxrow(struct mailboxrow *mailboxrow)
{

    return ((unsigned long)mailboxrow - (unsigned long)mailboxrows) / sizeof (struct mailboxrow);

}

struct core *pool_getcore(unsigned int icore)
{

    struct corerow *corerow = getcorerow(icore);

    return corerow ? &corerow->core : 0;

}

struct list_item *pool_getcoreitem(unsigned int icore)
{

    struct corerow *corerow = getcorerow(icore);

    return corerow ? &corerow->item : 0;

}

struct mailbox *pool_getmailbox(unsigned int imailbox)
{

    struct mailboxrow *mailboxrow = getmailboxrow(imailbox);

    return mailboxrow ? &mailboxrow->mailbox : 0;

}

struct list_item *pool_getmailboxitem(unsigned int imailbox)
{

    struct mailboxrow *mailboxrow = getmailboxrow(imailbox);

    return mailboxrow ? &mailboxrow->item : 0;

}

struct node *pool_getnode(unsigned int inode)
{

    struct noderow *noderow = getnoderow(inode);

    return noderow ? &noderow->node : 0;

}

struct list_item *pool_getnodeitem(unsigned int inode)
{

    struct noderow *noderow = getnoderow(inode);

    return noderow ? &noderow->item : 0;

}

struct task *pool_gettask(unsigned int itask)
{

    struct taskrow *taskrow = gettaskrow(itask);

    return taskrow ? &taskrow->task : 0;

}

struct list_item *pool_gettaskitem(unsigned int itask)
{

    struct taskrow *taskrow = gettaskrow(itask);

    return taskrow ? &taskrow->item : 0;

}

unsigned int pool_getinodefromitem(struct list_item *item)
{

    return encodenoderow(item->data);

}

unsigned int pool_getitaskfromitem(struct list_item *item)
{

    return encodetaskrow(item->data);

}

unsigned int pool_pickmailbox(unsigned int itask)
{

    struct list_item *item = list_pickhead(&freemailboxes);

    if (item)
    {

        struct mailboxrow *mailboxrow = item->data;
        struct mailbox *mailbox = &mailboxrow->mailbox;

        mailbox_reset(mailbox, itask);

        return encodemailboxrow(mailboxrow);

    }

    return 0;

}

unsigned int pool_picknode(char *name, struct resource *resource, struct node_operands *operands)
{

    struct list_item *item = list_pickhead(&freenodes);

    if (item)
    {

        struct noderow *noderow = item->data;
        struct node *node = &noderow->node;

        node_reset(node, name, resource, operands);

        return encodenoderow(noderow);

    }

    return 0;

}

unsigned int pool_picktask(void)
{

    struct list_item *item = list_pickhead(&freetasks);

    if (item)
    {

        struct taskrow *taskrow = item->data;
        struct task *task = &taskrow->task;

        task_reset(task);

        return encodetaskrow(taskrow);

    }

    return 0;

}

void pool_unpickmailbox(unsigned int imailbox)
{

    struct list_item *item = pool_getmailboxitem(imailbox);

    if (item)
        list_add(&freemailboxes, item);

}

void pool_unpicktask(unsigned int itask)
{

    struct list_item *item = pool_gettaskitem(itask);

    if (item)
        list_add(&freetasks, item);

}

void pool_addnode(struct list *nodes, unsigned int inode)
{

    struct list_item *item = pool_getnodeitem(inode);

    if (item)
        list_add(nodes, item);

}

void pool_removenode(struct list *nodes, unsigned int inode)
{

    struct list_item *item = pool_getnodeitem(inode);

    if (item)
        list_move(&freenodes, nodes, item);

}

unsigned int pool_findinode(unsigned int namehash, unsigned int index)
{

    unsigned int n = 0;
    unsigned int i;

    for (i = 0; i < POOL_NODES; i++)
    {

        struct noderow *noderow = getnoderow(i);

        if (noderow && noderow->node.namehash == namehash)
        {

            if (n == index)
                return encodenoderow(noderow);

            n++;

        }

    }

    return 0;

}

void pool_setup(unsigned long saddress, unsigned int ssize, unsigned long mbaddress, unsigned int mbsize)
{

    unsigned int i;

    list_init(&freetasks);
    list_init(&freemailboxes);
    list_init(&freenodes);
    list_init(&usednodes);

    for (i = 0; i < POOL_CORES; i++)
    {

        struct corerow *corerow = &corerows[i];

        core_init(&corerow->core, i, saddress + i * ssize + ssize);
        core_register(&corerow->core);
        list_inititem(&corerow->item, &corerow->core);

    }

    for (i = 1; i < POOL_NODES; i++)
    {

        struct noderow *noderow = &noderows[i];

        node_init(&noderow->node);
        list_inititem(&noderow->item, noderow);
        list_add(&freenodes, &noderow->item);

    }

    for (i = 1; i < POOL_MAILBOXES; i++)
    {

        struct mailboxrow *mailboxrow = &mailboxrows[i];

        mailbox_init(&mailboxrow->mailbox, (void *)(mbaddress + i * mbsize), mbsize);
        mailbox_register(&mailboxrow->mailbox);
        list_inititem(&mailboxrow->item, mailboxrow);
        list_add(&freemailboxes, &mailboxrow->item);

    }

    for (i = 1; i < POOL_TASKS; i++)
    {

        struct taskrow *taskrow = &taskrows[i];

        task_init(&taskrow->task);
        task_register(&taskrow->task);
        list_inititem(&taskrow->item, taskrow);
        list_add(&freetasks, &taskrow->item);

    }

}

