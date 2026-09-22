#include <fudge.h>
#include <kernel.h>

#define ROOTRECORDS                     4
#define GCORES                          0x01000
#define GTASKS                          0x04000
#define GMAILBOXES                      0x06000
#define GNODES                          0x08000

static struct node_operands operands;
static unsigned int inode;
static struct record rootrecords[ROOTRECORDS];

static unsigned int readcores(unsigned int id, unsigned int offset, unsigned int count, void *data)
{

    struct record *records = data;
    unsigned int nrecords = count / sizeof (struct record);
    struct resource *resource = 0;
    unsigned int c = 0;
    unsigned int i;

    for (i = 0; (resource = resource_foreachtype(resource, RESOURCE_CORE)); i++)
    {

        if (c >= nrecords)
            break;

        if (i >= offset)
        {

            struct core *core = resource->data;

            if (core && core->state != TASK_STATE_DEAD)
            {

                char name[RECORD_NAMESIZE];
                unsigned int cname = cstring_write_fmt1(name, RECORD_NAMESIZE, 0, "core%u", &i);

                record_init(&records[c], GCORES + i + 1, RECORD_TYPE_DIRECTORY, 0, i + 1, cname, name);

                c++;

            }

        }

    }

    return c * sizeof (struct record);

}

static unsigned int readtasks(unsigned int id, unsigned int offset, unsigned int count, void *data)
{

    struct record *records = data;
    unsigned int nrecords = count / sizeof (struct record);
    struct resource *resource = 0;
    unsigned int c = 0;
    unsigned int i;

    for (i = 0; (resource = resource_foreachtype(resource, RESOURCE_TASK)); i++)
    {

        if (c >= nrecords)
            break;

        if (i >= offset)
        {

            struct task *task = resource->data;

            if (task && task->state != TASK_STATE_DEAD)
            {

                char name[RECORD_NAMESIZE];
                unsigned int cname = cstring_write_fmt1(name, RECORD_NAMESIZE, 0, "task%u", &i);

                record_init(&records[c], GTASKS + i, RECORD_TYPE_DIRECTORY, 0, i + 1, cname, name);

                c++;

            }

        }

    }

    return c * sizeof (struct record);

}

static unsigned int readmailboxes(unsigned int id, unsigned int offset, unsigned int count, void *data)
{

    struct record *records = data;
    unsigned int nrecords = count / sizeof (struct record);
    struct resource *resource = 0;
    unsigned int c = 0;
    unsigned int i;

    for (i = 0; (resource = resource_foreachtype(resource, RESOURCE_MAILBOX)); i++)
    {

        if (c >= nrecords)
            break;

        if (i >= offset)
        {

            struct mailbox *mailbox = resource->data;

            if (mailbox && mailbox->itask)
            {

                char name[RECORD_NAMESIZE];
                unsigned int cname = cstring_write_fmt1(name, RECORD_NAMESIZE, 0, "mailbox%u", &i);

                record_init(&records[c], GMAILBOXES + i, RECORD_TYPE_DIRECTORY, 0, i + 1, cname, name);

                c++;

            }

        }

    }

    return c * sizeof (struct record);

}

static unsigned int readnodes(unsigned int id, unsigned int offset, unsigned int count, void *data)
{

    struct record *records = data;
    unsigned int nrecords = count / sizeof (struct record);
    struct resource *resource = 0;
    unsigned int c = 0;
    unsigned int i;

    for (i = 0; (resource = resource_foreachtype(resource, RESOURCE_NODE)); i++)
    {

        if (c >= nrecords)
            break;

        if (i >= offset)
        {

            struct node *node = resource->data;

            if (node && node->namehash)
            {

                char name[RECORD_NAMESIZE];
                unsigned int cname = cstring_write_fmt1(name, RECORD_NAMESIZE, 0, "node%u", &i);

                record_init(&records[c], GNODES + i, RECORD_TYPE_DIRECTORY, 0, i + 1, cname, name);

                c++;

            }

        }

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

    case 0x1002:
        return readtasks(id, offset, count, data);

    case 0x1003:
        return readmailboxes(id, offset, count, data);

    case 0x1004:
        return readnodes(id, offset, count, data);

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

        if (record->length == length - 1 && buffer_match(record->name, path, record->length))
            return record->id;

    }

    return 0;

}

static unsigned int walk(unsigned int parent, unsigned int length, char *path)
{

    if (!length)
        return parent;

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

    response.id = walk(request->parent ? request->parent : 1, request->length, (char *)(request + 1));

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
    record_init(&rootrecords[2], 0x1003, RECORD_TYPE_DIRECTORY, 0, 3, 9, "mailboxes");
    record_init(&rootrecords[3], 0x1004, RECORD_TYPE_DIRECTORY, 0, 4, 5, "nodes");
    node_operands_init(&operands, 0, operands_place);

    inode = pool_picknode();

    if (inode)
    {

        struct node *node = pool_getnode(inode);

        node_reset(node, "sysinfo", 0, &operands);

    }

}

