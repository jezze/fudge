#include <fudge.h>
#include <kernel.h>

#define ROOTRECORDS                     3

static struct node_operands operands;
static unsigned int inode;
static struct record rootrecords[ROOTRECORDS];

static unsigned int readcores(unsigned int id, unsigned int offset, unsigned int count, void *data)
{

    struct record *records = data;
    unsigned int nrecords = count / sizeof (struct record);
    struct resource *resource = (struct resource *)offset;
    unsigned int c;

    for (c = 0; (resource = resource_foreachtype(resource, RESOURCE_CORE)); c++)
    {

        if (c >= nrecords)
            break;

        record_init(&records[c], (unsigned int)resource, RECORD_TYPE_NORMAL, 0, (unsigned int)resource_foreachtype(resource, RESOURCE_CORE), 5, "core0");

    }

    return c * sizeof (struct record);

}

static unsigned int readroot(unsigned int id, unsigned int offset, unsigned int count, void *data)
{

    struct record *records = data;
    unsigned int nrecords = count / sizeof (struct record);
    unsigned int total = (nrecords < ROOTRECORDS) ? nrecords : ROOTRECORDS;
    unsigned int c = 0;
    unsigned int i;

    for (i = offset; i < total; i++)
    {

        buffer_copy(&records[c], &rootrecords[i], sizeof (struct record));

        c++;

    }

    return c * sizeof (struct record);

}

static unsigned int read(unsigned int id, unsigned int offset, unsigned int count, void *data)
{

    switch (id)
    {

    case 0x0001:
        return readroot(id, offset, count, data);

    case 0x1001:
        return readcores(id, offset, count, data);

    }

    return 0;

}

static unsigned int stat(unsigned int id, void *data)
{

    unsigned int group = id >> 12;
    unsigned int i;

    switch (group)
    {

    case 0:
        break;

    case 1:
        for (i = 0; i < ROOTRECORDS; i++)
        {

            struct record *current = &rootrecords[i];

            if (current->id == id)
            {

                buffer_copy(data, current, sizeof (struct record));

                return sizeof (struct record);

            }

        }

        break;

    }

    return 0;

}

static unsigned int walkroot(unsigned int parent, unsigned int length, char *path)
{

    unsigned int i;

    for (i = 0; i < ROOTRECORDS; i++)
    {

        struct record *record = &rootrecords[i];

        if (record->length == length && buffer_match(record->name, path, record->length))
            return record->id;

    }

    return 0;

}

static unsigned int walk(unsigned int parent, unsigned int length, char *path)
{

    switch (parent)
    {

    case 1:
        return walkroot(parent, length, path);

    }

    return 0;

}

static unsigned int onreadrequest(unsigned int source, unsigned int count, void *data)
{

    unsigned char buffer[MESSAGE_SIZE];
    struct event_readrequest *request = data;
    struct event_readresponse *response = (struct event_readresponse *)buffer;

    response->count = read(request->id, request->offset, request->count, response + 1);

    return kernel_place(inode, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + response->count, buffer);

}

static unsigned int onstatrequest(unsigned int source, unsigned int count, void *data)
{

    unsigned char buffer[MESSAGE_SIZE];
    struct event_statrequest *request = data;
    struct event_statresponse *response = (struct event_statresponse *)buffer;

    response->count = stat(request->id, response + 1);

    return kernel_place(inode, source, EVENT_STATRESPONSE, sizeof (struct event_statresponse) + response->count, response);

}

static unsigned int onwalkrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_walkrequest *request = data;
    struct event_walkresponse response;

    if (request->parent)
        response.id = walk(request->parent, request->length, (char *)(request + 1));
    else
        response.id = 1;

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

    record_init(&rootrecords[0], 0x1001, RECORD_TYPE_DIRECTORY, 0, 1, 5, "cores");
    record_init(&rootrecords[1], 0x1002, RECORD_TYPE_DIRECTORY, 0, 2, 5, "tasks");
    record_init(&rootrecords[2], 0x1003, RECORD_TYPE_DIRECTORY, 0, 3, 5, "nodes");
    node_operands_init(&operands, 0, operands_place);

    inode = pool_picknode();

    if (inode)
    {

        struct node *node = pool_getnode(inode);

        node_reset(node, "sysinfo", 0, &operands);

    }

}

