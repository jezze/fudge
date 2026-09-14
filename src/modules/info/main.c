#include <fudge.h>
#include <kernel.h>

static struct node_operands operands;
static unsigned int inode;

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

    unsigned int crecords = 0;

    if (offset < 1)
    {

        struct record *record = &records[crecords];

        record->id = 2;
        record->offset = 1;
        record->type = RECORD_TYPE_DIRECTORY;
        record->size = 0;
        record->length = 5;

        buffer_write(record->name, RECORD_NAMESIZE, "cores", record->length, 0);

        crecords++;

    }

    if (offset < 2)
    {

        struct record *record = &records[crecords];

        record->id = 3;
        record->offset = 2;
        record->type = RECORD_TYPE_DIRECTORY;
        record->size = 0;
        record->length = 5;

        buffer_write(record->name, RECORD_NAMESIZE, "tasks", record->length, 0);

        crecords++;

    }

    if (offset < 3)
    {

        struct record *record = &records[crecords];

        record->id = 4;
        record->offset = 3;
        record->type = RECORD_TYPE_DIRECTORY;
        record->size = 0;
        record->length = 5;

        buffer_write(record->name, RECORD_NAMESIZE, "nodes", record->length, 0);

        crecords++;

    }

    return crecords * sizeof (struct record);

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

void module_init(void)
{

    inode = pool_picknode();

    node_operands_init(&operands, 0, operands_place);

    if (inode)
    {

        struct node *node = pool_getnode(inode);

        node_reset(node, "sysinfo", 0, &operands);

    }

}

