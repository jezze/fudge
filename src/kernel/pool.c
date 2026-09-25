#include <fudge.h>
#include "resource.h"
#include "mailbox.h"
#include "core.h"
#include "task.h"
#include "node.h"
#include "service.h"
#include "pool.h"

static struct corerow {struct list_item item; struct core core;} corerows[POOL_CORES];
static struct taskrow {struct list_item item; struct task task;} taskrows[POOL_TASKS];
static struct mailboxrow {struct list_item item; struct mailbox mailbox;} mailboxrows[POOL_MAILBOXES];
static struct noderow {struct list_item item; struct resource *resource; struct node_operands *operands;} noderows[POOL_NODES];
static struct servicerow {struct list_item item; struct service service;} servicerows[POOL_SERVICES];
static struct list freetasks;
static struct list freemailboxes;
static struct list freenodes;
static struct list freeservices;

static struct corerow *getcorerow(unsigned int icore)
{

    return (icore < POOL_CORES) ? &corerows[icore] : 0;

}

static struct list_item *getcoreitem(unsigned int icore)
{

    struct corerow *corerow = getcorerow(icore);

    return corerow ? &corerow->item : 0;

}

static struct mailboxrow *getmailboxrow(unsigned int imailbox)
{

    return (imailbox && imailbox < POOL_MAILBOXES) ? &mailboxrows[imailbox] : 0;

}

static struct list_item *getmailboxitem(unsigned int imailbox)
{

    struct mailboxrow *mailboxrow = getmailboxrow(imailbox);

    return mailboxrow ? &mailboxrow->item : 0;

}

static struct noderow *getnoderow(unsigned int inode)
{

    return (inode && inode < POOL_NODES) ? &noderows[inode] : 0;

}

static struct servicerow *getservicerow(unsigned int iservice)
{

    return (iservice && iservice < POOL_NODES) ? &servicerows[iservice] : 0;

}

static struct list_item *getnodeitem(unsigned int inode)
{

    struct noderow *noderow = getnoderow(inode);

    return noderow ? &noderow->item : 0;

}

static struct list_item *getserviceitem(unsigned int iservice)
{

    struct servicerow *servicerow = getservicerow(iservice);

    return servicerow ? &servicerow->item : 0;

}

static struct taskrow *gettaskrow(unsigned int itask)
{

    return (itask && itask < POOL_TASKS) ? &taskrows[itask] : 0;

}

static struct list_item *gettaskitem(unsigned int itask)
{

    struct taskrow *taskrow = gettaskrow(itask);

    return taskrow ? &taskrow->item : 0;

}

static unsigned int encodecorerow(struct corerow *corerow)
{

    return ((unsigned long)corerow - (unsigned long)corerows) / sizeof (struct corerow);

}

static unsigned int encodemailboxrow(struct mailboxrow *mailboxrow)
{

    return ((unsigned long)mailboxrow - (unsigned long)mailboxrows) / sizeof (struct mailboxrow);

}

static unsigned int encodenoderow(struct noderow *noderow)
{

    return ((unsigned long)noderow - (unsigned long)noderows) / sizeof (struct noderow);

}

static unsigned int encodeservicerow(struct servicerow *servicerow)
{

    return ((unsigned long)servicerow - (unsigned long)servicerows) / sizeof (struct servicerow);

}

static unsigned int encodetaskrow(struct taskrow *taskrow)
{

    return ((unsigned long)taskrow - (unsigned long)taskrows) / sizeof (struct taskrow);

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

struct node_operands *pool_getnodeoperands(unsigned int inode)
{

    struct noderow *noderow = getnoderow(inode);

    return noderow ? noderow->operands : 0;

}

struct service *pool_getservice(unsigned int iservice)
{

    struct servicerow *servicerow = getservicerow(iservice);

    return servicerow ? &servicerow->service : 0;

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

unsigned int pool_getitaskfromitem(struct list_item *item)
{

    return encodetaskrow(item->data);

}

unsigned int pool_pickcorefrom(struct list *list)
{

    struct list_item *item = list_pickhead(list);

    return (item) ? encodecorerow(item->data) : 0;

}

unsigned int pool_pickmailbox(void)
{

    struct list_item *item = list_pickhead(&freemailboxes);

    return (item) ? encodemailboxrow(item->data) : 0;

}

unsigned int pool_picknode(struct resource *resource, struct node_operands *operands)
{

    struct list_item *item = list_pickhead(&freenodes);

    if (item)
    {

        struct noderow *noderow = item->data;

        noderow->resource = resource;
        noderow->operands = operands;

        return encodenoderow(item->data);

    }

    return 0;

}

unsigned int pool_pickservice(void)
{

    struct list_item *item = list_pickhead(&freeservices);

    return (item) ? encodeservicerow(item->data) : 0;

}

unsigned int pool_picktask(void)
{

    struct list_item *item = list_pickhead(&freetasks);

    return (item) ? encodetaskrow(item->data) : 0;

}

unsigned int pool_picktaskfrom(struct list *list)
{

    struct list_item *item = list_pickhead(list);

    return (item) ? encodetaskrow(item->data) : 0;

}

void pool_unpickmailbox(unsigned int imailbox)
{

    struct list_item *item = getmailboxitem(imailbox);

    if (item)
        list_add(&freemailboxes, item);

}

void pool_unpicknode(unsigned int inode)
{

    struct list_item *item = getnodeitem(inode);

    if (item)
        list_add(&freenodes, item);

}

void pool_unpickservice(unsigned int iservice)
{

    struct list_item *item = getserviceitem(iservice);

    if (item)
        list_add(&freeservices, item);

}

void pool_unpicktask(unsigned int itask)
{

    struct list_item *item = gettaskitem(itask);

    if (item)
        list_add(&freetasks, item);

}

void pool_placecore(unsigned int icore, struct list *list)
{

    struct list_item *item = getcoreitem(icore);

    if (item)
        list_add(list, item);

}

void pool_placenode(unsigned int inode, struct list *list)
{

    struct list_item *item = getnodeitem(inode);

    if (item)
        list_add(list, item);

}

void pool_placeservice(unsigned int iservice, struct list *list)
{

    struct list_item *item = getserviceitem(iservice);

    if (item)
        list_add(list, item);

}

void pool_placetask(unsigned int itask, struct list *list)
{

    struct list_item *item = gettaskitem(itask);

    if (item)
        list_add(list, item);

}

void pool_setup(unsigned long mbaddress)
{

    unsigned int i;

    list_init(&freetasks);
    list_init(&freemailboxes);
    list_init(&freenodes);
    list_init(&freeservices);

    for (i = 0; i < POOL_CORES; i++)
    {

        struct corerow *corerow = &corerows[i];

        core_init(&corerow->core);
        list_inititem(&corerow->item, corerow);

    }

    for (i = 1; i < POOL_NODES; i++)
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

    for (i = 1; i < POOL_SERVICES; i++)
    {

        struct servicerow *servicerow = &servicerows[i];

        service_init(&servicerow->service, 0);
        list_inititem(&servicerow->item, servicerow);
        list_add(&freeservices, &servicerow->item);

    }

    for (i = 1; i < POOL_TASKS; i++)
    {

        struct taskrow *taskrow = &taskrows[i];

        task_init(&taskrow->task);
        list_inititem(&taskrow->item, taskrow);
        list_add(&freetasks, &taskrow->item);

    }

}

