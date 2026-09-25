#include <fudge.h>
#include <kernel.h>
#include <modules/base/bus.h>
#include <modules/base/driver.h>

#define ROOTRECORDS                     7

static struct node_operands operands;
static struct service service;
static unsigned int inode;
static struct record rootrecords[ROOTRECORDS];

static unsigned int readcores(unsigned int id, unsigned int offset, unsigned int count, void *data)
{

    struct resource *resource = 0;
    unsigned int c = 0;
    unsigned int i;
    char buffer[4096];
    char *states[3] = {
        "UNKNOWN",
        "DEAD",
        "ACTIVE"
    };

    c += cstring_write_fmt0(buffer, 4096, c, "cores: [\n");

    for (i = 0; (resource = resource_foreachtype(resource, RESOURCE_CORE)); i++)
    {

        struct core *core = resource->data;

        c += cstring_write_fmt0(buffer, 4096, c, "  {\n");
        c += cstring_write_fmt1(buffer, 4096, c, "    id: %u\n", &i);
        c += cstring_write_fmt1(buffer, 4096, c, "    state: %s\n", states[core->state]);
        c += cstring_write_fmt1(buffer, 4096, c, "    tasks: %u\n", &core->tasks.count);
        c += cstring_write_fmt1(buffer, 4096, c, "    running: %u\n", &core->itask);
        c += cstring_write_fmt0(buffer, 4096, c, "  }\n");

    }

    c += cstring_write_fmt0(buffer, 4096, c, "]\n");

    return buffer_read(data, count, buffer, c, offset);

}

static unsigned int readtasks(unsigned int id, unsigned int offset, unsigned int count, void *data)
{

    struct resource *resource = 0;
    unsigned int c = 0;
    unsigned int i;
    char buffer[4096];
    char *states[7] = {
        "UNKNOWN",
        "DEAD",
        "NEW",
        "BLOCKED",
        "UNBLOCKED",
        "ASSIGNED",
        "RUNNING"
    };

    c += cstring_write_fmt0(buffer, 4096, c, "tasks: [\n");

    for (i = 0; (resource = resource_foreachtype(resource, RESOURCE_TASK)); i++)
    {

        struct task *task = resource->data;

        c += cstring_write_fmt0(buffer, 4096, c, "  {\n");
        c += cstring_write_fmt1(buffer, 4096, c, "    id: %u\n", &i);
        c += cstring_write_fmt1(buffer, 4096, c, "    state: %s\n", states[task->state]);
        c += cstring_write_fmt1(buffer, 4096, c, "    address: 0x%H8u\n", &task->address);
        c += cstring_write_fmt0(buffer, 4096, c, "    signals:\n");
        c += cstring_write_fmt0(buffer, 4096, c, "      {\n");
        c += cstring_write_fmt1(buffer, 4096, c, "        kill: %u\n", &task->signals.kill);
        c += cstring_write_fmt1(buffer, 4096, c, "        block: %u\n", &task->signals.block);
        c += cstring_write_fmt1(buffer, 4096, c, "        unblock: %u\n", &task->signals.unblock);
        c += cstring_write_fmt0(buffer, 4096, c, "      }\n");
        c += cstring_write_fmt0(buffer, 4096, c, "  }\n");

    }

    c += cstring_write_fmt0(buffer, 4096, c, "]\n");

    return buffer_read(data, count, buffer, c, offset);

}

static unsigned int readmailboxes(unsigned int id, unsigned int offset, unsigned int count, void *data)
{

    struct resource *resource = 0;
    unsigned int c = 0;
    char buffer[4096];

    c += cstring_write_fmt0(buffer, 4096, c, "mailboxes: [\n");

    while ((resource = resource_foreachtype(resource, RESOURCE_MAILBOX)))
    {

        struct mailbox *mailbox = resource->data;
        unsigned int nmessages = mailbox->head - mailbox->tail;

        c += cstring_write_fmt0(buffer, 4096, c, "  {\n");
        c += cstring_write_fmt1(buffer, 4096, c, "    itask: %u\n", &mailbox->itask);
        c += cstring_write_fmt1(buffer, 4096, c, "    ichannel: %u\n", &mailbox->ichannel);
        c += cstring_write_fmt1(buffer, 4096, c, "    inode: %u\n", &mailbox->inode);
        c += cstring_write_fmt1(buffer, 4096, c, "    nmessages: %u\n", &nmessages);
        c += cstring_write_fmt0(buffer, 4096, c, "  }\n");

    }

    c += cstring_write_fmt0(buffer, 4096, c, "]\n");

    return buffer_read(data, count, buffer, c, offset);

}

static unsigned int readnodes(unsigned int id, unsigned int offset, unsigned int count, void *data)
{

    return 0;

}

static unsigned int readbuses(unsigned int id, unsigned int offset, unsigned int count, void *data)
{

    struct resource *resource = 0;
    unsigned int c = 0;
    char buffer[4096];

    c += cstring_write_fmt0(buffer, 4096, c, "buses: [\n");

    while ((resource = resource_foreachtype(resource, RESOURCE_BUS)))
    {

        struct base_bus *bus = resource->data;

        c += cstring_write_fmt0(buffer, 4096, c, "  {\n");
        c += cstring_write_fmt1(buffer, 4096, c, "    name: %s\n", bus->name);
        c += cstring_write_fmt0(buffer, 4096, c, "  }\n");

    }

    c += cstring_write_fmt0(buffer, 4096, c, "]\n");

    return buffer_read(data, count, buffer, c, offset);

}

static unsigned int readdrivers(unsigned int id, unsigned int offset, unsigned int count, void *data)
{

    struct resource *resource = 0;
    unsigned int c = 0;
    char buffer[4096];

    c += cstring_write_fmt0(buffer, 4096, c, "drivers: [\n");

    while ((resource = resource_foreachtype(resource, RESOURCE_DRIVER)))
    {

        struct base_driver *driver = resource->data;

        c += cstring_write_fmt0(buffer, 4096, c, "  {\n");
        c += cstring_write_fmt1(buffer, 4096, c, "    name: %s\n", driver->name);
        c += cstring_write_fmt0(buffer, 4096, c, "  }\n");

    }

    c += cstring_write_fmt0(buffer, 4096, c, "]\n");

    return buffer_read(data, count, buffer, c, offset);

}

static unsigned int readservices(unsigned int id, unsigned int offset, unsigned int count, void *data)
{

    struct resource *resource = 0;
    unsigned int c = 0;
    char buffer[4096];

    c += cstring_write_fmt0(buffer, 4096, c, "services: [\n");

    while ((resource = resource_foreachtype(resource, RESOURCE_SERVICE)))
    {

        struct service *service = resource->data;

        c += cstring_write_fmt0(buffer, 4096, c, "  {\n");
        c += cstring_write_fmt1(buffer, 4096, c, "    name: %s\n", service->name);
        c += cstring_write_fmt1(buffer, 4096, c, "    inode: %u\n", &service->inode);
        c += cstring_write_fmt0(buffer, 4096, c, "  }\n");

    }

    c += cstring_write_fmt0(buffer, 4096, c, "]\n");

    return buffer_read(data, count, buffer, c, offset);

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

    case 0x1005:
        return readbuses(id, offset, count, data);

    case 0x1006:
        return readdrivers(id, offset, count, data);

    case 0x1007:
        return readservices(id, offset, count, data);

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

        switch (record->type)
        {

        case RECORD_TYPE_NORMAL:
            if (record->length == length && buffer_match(record->name, path, record->length))
                return record->id;

            break;

        case RECORD_TYPE_DIRECTORY:
            if (record->length == length - 1 && buffer_match(record->name, path, record->length))
                return record->id;

            break;

        }

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

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
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

    record_init(&rootrecords[0], 0x1001, RECORD_TYPE_NORMAL, 0, 1, 5, "cores");
    record_init(&rootrecords[1], 0x1002, RECORD_TYPE_NORMAL, 0, 2, 5, "tasks");
    record_init(&rootrecords[2], 0x1003, RECORD_TYPE_NORMAL, 0, 3, 9, "mailboxes");
    record_init(&rootrecords[3], 0x1004, RECORD_TYPE_NORMAL, 0, 4, 5, "nodes");
    record_init(&rootrecords[4], 0x1005, RECORD_TYPE_NORMAL, 0, 5, 5, "buses");
    record_init(&rootrecords[5], 0x1006, RECORD_TYPE_NORMAL, 0, 6, 7, "drivers");
    record_init(&rootrecords[6], 0x1007, RECORD_TYPE_NORMAL, 0, 7, 8, "services");
    node_operands_init(&operands, 0, operands_place);

    inode = pool_picknode();

    if (inode)
    {

        struct node *node = pool_getnode(inode);

        node_reset(node, 0, &operands);
        service_init(&service, "sysinfo");
        service_register(&service, inode);

    }

}

