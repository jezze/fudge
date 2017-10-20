#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct system_node root;
static struct system_node idata;
static struct system_node odata;
static struct list idatalist;
static struct spinlock idatalock;
static struct list odatalist;
static struct spinlock odatalock;
static struct ring ring;
static char data[FUDGE_BSIZE];
static struct spinlock datalock;

static unsigned int idata_open(struct system_node *self, struct service_state *state)
{

    list_lockadd(&idatalist, &state->item, &idatalock);
    spinlock_acquire(&odatalock);
    kernel_unblockall(&odatalist);
    spinlock_release(&odatalock);

    return (unsigned int)self;

}

static unsigned int idata_close(struct system_node *self, struct service_state *state)
{

    list_lockremove(&idatalist, &state->item, &idatalock);
    spinlock_acquire(&odatalock);
    kernel_unblockall(&odatalist);
    spinlock_release(&odatalock);

    return (unsigned int)self;

}

static unsigned int idata_read(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    spinlock_acquire(&datalock);

    count = ring_read(&ring, buffer, count);

    spinlock_release(&datalock);
    spinlock_acquire(&odatalock);
    kernel_unblockall(&odatalist);
    spinlock_release(&odatalock);

    if (!count && odatalist.count)
        kernel_blocktask(state->task);

    return count;

}

static unsigned int odata_open(struct system_node *self, struct service_state *state)
{

    list_lockadd(&odatalist, &state->item, &odatalock);
    spinlock_acquire(&idatalock);
    kernel_unblockall(&idatalist);
    spinlock_release(&idatalock);

    return (unsigned int)self;

}

static unsigned int odata_close(struct system_node *self, struct service_state *state)
{

    list_lockremove(&odatalist, &state->item, &odatalock);
    spinlock_acquire(&idatalock);
    kernel_unblockall(&idatalist);
    spinlock_release(&idatalock);

    return (unsigned int)self;

}

static unsigned int odata_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    spinlock_acquire(&datalock);

    count = ring_write(&ring, buffer, count);

    spinlock_release(&datalock);
    spinlock_acquire(&idatalock);
    kernel_unblockall(&idatalist);
    spinlock_release(&idatalock);

    if (!count)
        kernel_blocktask(state->task);

    return count;

}

void module_init(void)
{

    ring_init(&ring, FUDGE_BSIZE, data);
    system_initnode(&root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "buf");
    system_initnode(&idata, SYSTEM_NODETYPE_NORMAL, "idata");
    system_initnode(&odata, SYSTEM_NODETYPE_NORMAL, "odata");

    idata.open = idata_open;
    idata.close = idata_close;
    idata.read = idata_read;
    odata.open = odata_open;
    odata.close = odata_close;
    odata.write = odata_write;

    system_addchild(&root, &idata);
    system_addchild(&root, &odata);

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

