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

static unsigned int idata_open(struct system_node *self, struct service_state *state)
{

    list_add(&idatalist, &state->item);
    kernel_unblockall(&odatalist);

    return (unsigned int)self;

}

static unsigned int idata_close(struct system_node *self, struct service_state *state)
{

    list_remove(&idatalist, &state->item);
    kernel_unblockall(&odatalist);

    return (unsigned int)self;

}

static unsigned int idata_read(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    count = ring_read(&ring, buffer, count);

    kernel_unblockall(&odatalist);

    if (!count && odatalist.count)
        kernel_blocktask(state->task);

    return count;

}

static unsigned int odata_open(struct system_node *self, struct service_state *state)
{

    list_add(&odatalist, &state->item);
    kernel_unblockall(&idatalist);

    return (unsigned int)self;

}

static unsigned int odata_close(struct system_node *self, struct service_state *state)
{

    list_remove(&odatalist, &state->item);
    kernel_unblockall(&idatalist);

    return (unsigned int)self;

}

static unsigned int odata_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    count = ring_write(&ring, buffer, count);

    kernel_unblockall(&idatalist);

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

