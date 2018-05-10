#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct system_node root;
static struct system_node idata;
static struct system_node odata;
static struct list idatalist;
static struct list odatalist;
static struct ring ring;
static char data[FUDGE_BSIZE];
static struct spinlock datalock;

static void unblock(struct list *states)
{

    struct list_item *current;

    spinlock_acquire(&states->spinlock);

    for (current = states->head; current; current = current->next)
    {

        struct service_state *state = current->data;

        kernel_unblocktask(state->task);

    }

    spinlock_release(&states->spinlock);

}

static struct system_node *idata_open(struct system_node *self, struct service_state *state)
{

    list_add(&idatalist, &state->item);

    return self;

}

static struct system_node *idata_close(struct system_node *self, struct service_state *state)
{

    list_remove(&idatalist, &state->item);
    unblock(&odatalist);

    return self;

}

static unsigned int idata_read(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    spinlock_acquire(&datalock);

    count = ring_read(&ring, buffer, count);

    spinlock_release(&datalock);
    unblock(&odatalist);

    if (!count && odatalist.count)
        kernel_blocktask(state->task);

    return count;

}

static struct system_node *odata_open(struct system_node *self, struct service_state *state)
{

    list_add(&odatalist, &state->item);

    return self;

}

static struct system_node *odata_close(struct system_node *self, struct service_state *state)
{

    list_remove(&odatalist, &state->item);
    unblock(&idatalist);

    return self;

}

static unsigned int odata_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    spinlock_acquire(&datalock);

    count = ring_write(&ring, buffer, count);

    spinlock_release(&datalock);
    unblock(&idatalist);

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

    idata.operations.open = idata_open;
    idata.operations.close = idata_close;
    idata.operations.read = idata_read;
    odata.operations.open = odata_open;
    odata.operations.close = odata_close;
    odata.operations.write = odata_write;

}

void module_register(void)
{

    system_registernode(&root);
    system_addchild(&root, &idata);
    system_addchild(&root, &odata);

}

void module_unregister(void)
{

    system_unregisternode(&root);
    system_removechild(&root, &idata);
    system_removechild(&root, &odata);

}

