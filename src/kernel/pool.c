#include <fudge.h>
#include "resource.h"
#include "mailbox.h"
#include "task.h"
#include "node.h"
#include "pool.h"

static struct taskrow {struct list_item item; struct task task;} taskrows[POOL_TASKS];
static struct mailboxrow {struct list_item item; struct mailbox mailbox;} mailboxrows[POOL_MAILBOXES];
static struct noderow {struct list_item item; struct node node;} noderows[POOL_NODES];
static struct list freetasks;
static struct list freemailboxes;
static struct list freenodes;
static struct list usedmailboxes;
static struct list usednodes;

static void *pickrow(struct list *from, struct list *to)
{

    struct list_item *item = list_pickhead(from);

    if (item)
    {

        list_add(to, item);

        return item->data;

    }

    return 0;

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

    return ((unsigned int)noderow - (unsigned int)noderows) / sizeof (struct noderow);

}

static unsigned int encodetaskrow(struct taskrow *taskrow)
{

    return ((unsigned int)taskrow - (unsigned int)taskrows) / sizeof (struct taskrow);

}

static unsigned int encodemailboxrow(struct mailboxrow *mailboxrow)
{

    return ((unsigned int)mailboxrow - (unsigned int)mailboxrows) / sizeof (struct mailboxrow);

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

unsigned int pool_getinodefromitem(struct list_item *item)
{

    struct noderow *noderow = item->data;

    return encodenoderow(noderow);

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

unsigned int pool_getitaskfromitem(struct list_item *item)
{

    struct taskrow *taskrow = item->data;

    return encodetaskrow(taskrow);

}

struct mailbox *pool_getmailbox(unsigned int imailbox)
{

    struct mailboxrow *mailboxrow = getmailboxrow(imailbox);

    return mailboxrow ? &mailboxrow->mailbox : 0;

}

unsigned int pool_addnodeto(struct list *nodes, char *name, struct resource *resource, struct node_operands *operands)
{

    struct noderow *noderow = pickrow(&freenodes, nodes);

    if (noderow)
    {

        node_reset(&noderow->node, name, resource, operands);

        return encodenoderow(noderow);

    }

    return 0;

}

unsigned int pool_addnode(char *name, struct resource *resource, struct node_operands *operands)
{

    return pool_addnodeto(&usednodes, name, resource, operands);

}

void pool_removenodefrom(struct list *nodes, unsigned int inode)
{

    struct list_item *item = pool_getnodeitem(inode);

    if (item)
        list_move(&freenodes, nodes, item);

}

unsigned int pool_addmailbox(unsigned int itask)
{

    struct mailboxrow *mailboxrow = pickrow(&freemailboxes, &usedmailboxes);

    if (mailboxrow)
    {

        struct mailbox *mailbox = &mailboxrow->mailbox;

        mailbox_reset(mailbox, itask);

        return encodemailboxrow(mailboxrow);

    }

    return 0;

}

void pool_removemailboxes(unsigned int itask)
{

    unsigned int i;

    for (i = 0; i < POOL_MAILBOXES; i++)
    {

        struct mailboxrow *mailboxrow = &mailboxrows[i];
        struct mailbox *mailbox = &mailboxrow->mailbox;

        if (mailbox->itask == itask)
        {

            list_move(&freemailboxes, &usedmailboxes, &mailboxrow->item);

        }

    }

}

unsigned int pool_findinode(unsigned int namehash, unsigned int index)
{

    unsigned int n = 0;
    unsigned int i;

    for (i = 0; i < POOL_NODES; i++)
    {

        struct noderow *noderow = &noderows[i];

        if (noderow->node.namehash == namehash)
        {

            if (n == index)
                return encodenoderow(noderow);

            n++;

        }

    }

    return 0;

}

unsigned int pool_createtask(void)
{

    struct list_item *taskrowitem = list_pickhead(&freetasks);

    if (taskrowitem)
    {

        struct taskrow *taskrow = taskrowitem->data;
        struct task *task = &taskrow->task;

        task_reset(task);

        return encodetaskrow(taskrow);

    }

    return 0;

}

void pool_destroytask(unsigned int itask)
{

    struct task *task = pool_gettask(itask);

    if (task)
    {

        struct list_item *item = pool_gettaskitem(itask);

        pool_removemailboxes(itask);
        list_add(&freetasks, item);

    }

}

void pool_setup(unsigned int mbaddress, unsigned int mbsize)
{

    unsigned int i;

    list_init(&freetasks);
    list_init(&freemailboxes);
    list_init(&freenodes);
    list_init(&usedmailboxes);
    list_init(&usednodes);

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

