#include <fudge.h>
#include <kernel.h>

static struct node_operands operands;
static unsigned int inode;
static struct record rootrecords[3];

static unsigned int readcores(unsigned int id, unsigned int offset, unsigned int count, unsigned int nrecords, struct record *records)
{

    struct resource *resource = 0;
    unsigned int i;

    for (i = 0; (resource = resource_foreachtype(resource, RESOURCE_CORE)); i++)
    {

    }

    return 0;

}

static unsigned int readroot(unsigned int id, unsigned int offset, unsigned int count, unsigned int nrecords, struct record *records)
{

    unsigned int total = (nrecords < 3) ? nrecords : 3;
    unsigned int c = 0;
    unsigned int i;

    for (i = offset; i < total; i++)
    {

        buffer_copy(&records[c], &rootrecords[i], sizeof (struct record));

        c++;

    }

    return c * sizeof (struct record);

}

static unsigned int read(unsigned int id, unsigned int offset, unsigned int count, unsigned int nrecords, struct record *records)
{

    switch (id)
    {

    case 1:
        return readroot(id, offset, count, nrecords, records);

    case 2:
        return readcores(id, offset, count, nrecords, records);

    }

    return 0;

}

static unsigned int stat(unsigned int id)
{

    return 0;

}

static unsigned int walk(unsigned int parent)
{

    return 1;

}

static unsigned int onreadrequest(unsigned int source, unsigned int count, void *data)
{

    unsigned char buffer[MESSAGE_SIZE];
    struct event_readrequest *request = data;
    struct event_readresponse *response = (struct event_readresponse *)buffer;

    response->count = read(request->id, request->offset, request->count, (MESSAGE_SIZE - sizeof (struct event_readresponse)) / sizeof (struct record), (struct record *)(response + 1));

    return kernel_place(inode, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + response->count, buffer);

}

static unsigned int onstatrequest(unsigned int source, unsigned int count, void *data)
{

    unsigned char buffer[MESSAGE_SIZE];
    struct event_statrequest *request = data;
    struct event_statresponse *response = (struct event_statresponse *)buffer;

    response->count = stat(request->id);

    return kernel_place(inode, source, EVENT_STATRESPONSE, sizeof (struct event_statresponse) + response->count, response);

}

static unsigned int onwalkrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_walkrequest *request = data;
    struct event_walkresponse response;

    response.id = walk(request->parent);;

    return kernel_place(inode, source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

}

static unsigned int operands_place(struct resource *resource, unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_READREQUEST:
        return onreadrequest(source, count, data);

    case EVENT_STATREQUEST:
        return onstatrequest(source, count, data);

    case EVENT_WALKREQUEST:
        return onwalkrequest(source, count, data);

    }

    return MESSAGE_FAILED;

}

static void record_init(struct record *record, unsigned int id, unsigned int type, unsigned int size, unsigned int offset, unsigned int length, char *name)
{

    record->id = id;
    record->type = type;
    record->size = size;
    record->offset = offset;
    record->length = length;

    buffer_write(record->name, RECORD_NAMESIZE, name, record->length, 0);

}

void module_init(void)
{

    record_init(&rootrecords[0], 1001, RECORD_TYPE_DIRECTORY, 0, 1, 5, "cores");
    record_init(&rootrecords[1], 1002, RECORD_TYPE_DIRECTORY, 0, 2, 5, "tasks");
    record_init(&rootrecords[2], 1003, RECORD_TYPE_DIRECTORY, 0, 3, 5, "nodes");
    node_operands_init(&operands, 0, operands_place);

    inode = pool_picknode();

    if (inode)
    {

        struct node *node = pool_getnode(inode);

        node_reset(node, "sysinfo", 0, &operands);

    }

}

