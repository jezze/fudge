#include <fudge.h>
#include <kernel.h>
#include <modules/base/bus.h>
#include <modules/base/driver.h>

#define ROOT                            0x0001
#define ROOTRECORDS                     8
#define OUTPUTSIZE                      0x4000

static struct node_operands operands;
static struct service service;
static unsigned int inode;
static struct record rootrecords[ROOTRECORDS];
static struct spinlock spinlock;
static char output[OUTPUTSIZE];
static unsigned char response[MESSAGE_SIZE];
static unsigned int outputcount;
static unsigned int outputid;
static unsigned int outputsource;

static unsigned int writecores(void)
{

    struct resource *resource = 0;
    unsigned int c = 0;
    unsigned int i;
    char *states[3] = {
        "UNKNOWN",
        "DEAD",
        "ACTIVE"
    };

    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "{\n");
    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "  cores: [\n");

    resource_lock();

    for (i = 0; (resource = resource_foreachtype_unsafe(resource, RESOURCE_CORE)); i++)
    {

        struct core *core = resource->data;

        c += cstring_write_fmt0(output, OUTPUTSIZE, c, "    {\n");
        c += cstring_write_fmt1(output, OUTPUTSIZE, c, "      id: %u\n", &i);
        c += cstring_write_fmt1(output, OUTPUTSIZE, c, "      state: %s\n", states[core->state]);
        c += cstring_write_fmt1(output, OUTPUTSIZE, c, "      tasks: %u\n", &core->tasks.count);
        c += cstring_write_fmt1(output, OUTPUTSIZE, c, "      running: %u\n", &core->itask);
        c += cstring_write_fmt0(output, OUTPUTSIZE, c, "    }\n");

    }

    resource_unlock();

    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "  ]\n");
    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "}\n");

    return c;

}

static unsigned int writetasks(void)
{

    struct resource *resource = 0;
    unsigned int c = 0;
    unsigned int i;
    char *states[7] = {
        "UNKNOWN",
        "DEAD",
        "NEW",
        "BLOCKED",
        "UNBLOCKED",
        "ASSIGNED",
        "RUNNING"
    };

    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "{\n");
    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "  tasks: [\n");

    resource_lock();

    for (i = 0; (resource = resource_foreachtype_unsafe(resource, RESOURCE_TASK)); i++)
    {

        struct task *task = resource->data;

        c += cstring_write_fmt0(output, OUTPUTSIZE, c, "    {\n");
        c += cstring_write_fmt1(output, OUTPUTSIZE, c, "      id: %u\n", &i);
        c += cstring_write_fmt1(output, OUTPUTSIZE, c, "      state: %s\n", states[task->state]);
        c += cstring_write_fmt1(output, OUTPUTSIZE, c, "      address: 0x%H8u\n", &task->address);
        c += cstring_write_fmt0(output, OUTPUTSIZE, c, "      signals:\n");
        c += cstring_write_fmt0(output, OUTPUTSIZE, c, "        {\n");
        c += cstring_write_fmt1(output, OUTPUTSIZE, c, "          kill: %u\n", &task->signals.kill);
        c += cstring_write_fmt1(output, OUTPUTSIZE, c, "          block: %u\n", &task->signals.block);
        c += cstring_write_fmt1(output, OUTPUTSIZE, c, "          unblock: %u\n", &task->signals.unblock);
        c += cstring_write_fmt0(output, OUTPUTSIZE, c, "        }\n");
        c += cstring_write_fmt0(output, OUTPUTSIZE, c, "    }\n");

    }

    resource_unlock();

    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "  ]\n");
    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "}\n");

    return c;

}

static unsigned int writemailboxes(void)
{

    struct resource *resource = 0;
    unsigned int c = 0;

    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "{\n");
    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "  mailboxes: [\n");

    resource_lock();

    while ((resource = resource_foreachtype_unsafe(resource, RESOURCE_MAILBOX)))
    {

        struct mailbox *mailbox = resource->data;
        unsigned int nmessages = mailbox->head - mailbox->tail;

        c += cstring_write_fmt0(output, OUTPUTSIZE, c, "    {\n");
        c += cstring_write_fmt1(output, OUTPUTSIZE, c, "      itask: %u\n", &mailbox->itask);
        c += cstring_write_fmt1(output, OUTPUTSIZE, c, "      ichannel: %u\n", &mailbox->ichannel);
        c += cstring_write_fmt1(output, OUTPUTSIZE, c, "      inode: %u\n", &mailbox->inode);
        c += cstring_write_fmt1(output, OUTPUTSIZE, c, "      nmessages: %u\n", &nmessages);
        c += cstring_write_fmt0(output, OUTPUTSIZE, c, "    }\n");

    }

    resource_unlock();

    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "  ]\n");
    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "}\n");

    return c;

}

static unsigned int writebuses(void)
{

    struct resource *resource = 0;
    unsigned int c = 0;

    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "{\n");
    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "  buses: [\n");

    resource_lock();

    while ((resource = resource_foreachtype_unsafe(resource, RESOURCE_BUS)))
    {

        struct base_bus *bus = resource->data;

        c += cstring_write_fmt0(output, OUTPUTSIZE, c, "    {\n");
        c += cstring_write_fmt1(output, OUTPUTSIZE, c, "      name: %s\n", bus->name);
        c += cstring_write_fmt0(output, OUTPUTSIZE, c, "    }\n");

    }

    resource_unlock();

    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "  ]\n");
    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "}\n");

    return c;

}

static unsigned int writedrivers(void)
{

    struct resource *resource = 0;
    unsigned int c = 0;

    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "{\n");
    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "  drivers: [\n");

    resource_lock();

    while ((resource = resource_foreachtype_unsafe(resource, RESOURCE_DRIVER)))
    {

        struct base_driver *driver = resource->data;

        c += cstring_write_fmt0(output, OUTPUTSIZE, c, "    {\n");
        c += cstring_write_fmt1(output, OUTPUTSIZE, c, "      name: %s\n", driver->name);
        c += cstring_write_fmt0(output, OUTPUTSIZE, c, "    }\n");

    }

    resource_unlock();

    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "  ]\n");
    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "}\n");

    return c;

}

static unsigned int writeservices(void)
{

    struct resource *resource = 0;
    unsigned int c = 0;

    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "{\n");
    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "  services: [\n");

    resource_lock();

    while ((resource = resource_foreachtype_unsafe(resource, RESOURCE_SERVICE)))
    {

        struct service *service = resource->data;

        c += cstring_write_fmt0(output, OUTPUTSIZE, c, "    {\n");
        c += cstring_write_fmt1(output, OUTPUTSIZE, c, "      name: %s\n", service->name);
        c += cstring_write_fmt1(output, OUTPUTSIZE, c, "      inode: %u\n", &service->inode);
        c += cstring_write_fmt0(output, OUTPUTSIZE, c, "    }\n");

    }

    resource_unlock();

    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "  ]\n");
    c += cstring_write_fmt0(output, OUTPUTSIZE, c, "}\n");

    return c;

}

static unsigned int readroot(unsigned int id, unsigned int offset, unsigned int count, void *data)
{

    struct record *records = data;
    unsigned int nrecords = count / sizeof (struct record);
    unsigned int c = 0;
    unsigned int i;

    for (i = offset; i < ROOTRECORDS && c < nrecords; i++)
    {

        buffer_copy(&records[c], &rootrecords[i], sizeof (struct record));

        c++;

    }

    return c * sizeof (struct record);

}

static unsigned int write(unsigned int id)
{

    switch (id)
    {

    case 0x1001:
        return writecores();

    case 0x1002:
        return writetasks();

    case 0x1003:
        return writemailboxes();

    case 0x1004:
        return writeservices();

    case 0x1005:
        return writebuses();

    case 0x1006:
        return writedrivers();

    }

    return 0;

}

static unsigned int read(unsigned int source, unsigned int id, unsigned int offset, unsigned int count, void *data)
{

    if (id == ROOT)
        return readroot(id, offset, count, data);

    /* a snapshot per reader, so a file read in several requests doesn't change between them */
    if (!offset || id != outputid || source != outputsource)
    {

        outputcount = write(id);
        outputid = id;
        outputsource = source;

    }

    return buffer_read(data, count, output, outputcount, offset);

}

static unsigned int stat(unsigned int id, void *data)
{

    unsigned int group = id >> 12;
    unsigned int i;

    switch (group)
    {

    case 0:
        if (id == ROOT)
        {

            buffer_copy(data, &rootrecords[0], sizeof (struct record));

            return sizeof (struct record);

        }

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

static unsigned int walkroot(char *name, unsigned int length)
{

    unsigned int i;

    for (i = 2; i < ROOTRECORDS; i++)
    {

        struct record *record = &rootrecords[i];

        if (record->length == length && buffer_match(record->name, name, length))
            return record->id;

    }

    return 0;

}

static unsigned int walk(unsigned int id, unsigned int length, char *path)
{

    unsigned int offset = 0;

    while (offset < length)
    {

        char *cp = path + offset;
        unsigned int cl = buffer_findbyte(cp, length - offset, '/');

        if (cl == 2 && cp[0] == '.' && cp[1] == '.')
            id = ROOT;
        else if (cl && (cl != 1 || cp[0] != '.'))
            id = (id == ROOT) ? walkroot(cp, cl) : 0;

        if (!id)
            return 0;

        offset += cl + 1;

    }

    return id;

}

static unsigned int onreadrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_readrequest *request = data;
    struct event_readresponse *readresponse = (struct event_readresponse *)response;
    unsigned int status;

    spinlock_acquire(&spinlock);

    readresponse->count = read(source, request->id, request->offset, (request->count < MESSAGE_SIZE - sizeof (struct event_readresponse)) ? request->count : MESSAGE_SIZE - sizeof (struct event_readresponse), readresponse + 1);
    status = kernel_place(inode, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + readresponse->count, response);

    spinlock_release(&spinlock);

    return status;

}

static unsigned int onstatrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_statrequest *request = data;
    struct event_statresponse *statresponse = (struct event_statresponse *)response;
    unsigned int status;

    spinlock_acquire(&spinlock);

    statresponse->count = stat(request->id, statresponse + 1);
    status = kernel_place(inode, source, EVENT_STATRESPONSE, sizeof (struct event_statresponse) + statresponse->count, response);

    spinlock_release(&spinlock);

    return status;

}

static unsigned int onwalkrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_walkrequest *request = data;
    struct event_walkresponse response;

    response.id = walk(request->parent ? request->parent : ROOT, request->length, (char *)(request + 1));

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

    record_init(&rootrecords[0], ROOT, RECORD_TYPE_DIRECTORY, 0, 1, 1, ".");
    record_init(&rootrecords[1], ROOT, RECORD_TYPE_DIRECTORY, 0, 2, 2, "..");
    record_init(&rootrecords[2], 0x1001, RECORD_TYPE_NORMAL, 0, 3, 5, "cores");
    record_init(&rootrecords[3], 0x1002, RECORD_TYPE_NORMAL, 0, 4, 5, "tasks");
    record_init(&rootrecords[4], 0x1003, RECORD_TYPE_NORMAL, 0, 5, 9, "mailboxes");
    record_init(&rootrecords[5], 0x1004, RECORD_TYPE_NORMAL, 0, 6, 8, "services");
    record_init(&rootrecords[6], 0x1005, RECORD_TYPE_NORMAL, 0, 7, 5, "buses");
    record_init(&rootrecords[7], 0x1006, RECORD_TYPE_NORMAL, 0, 8, 7, "drivers");
    spinlock_init(&spinlock);
    node_operands_init(&operands, 0, operands_place);
    service_init(&service);

    inode = pool_picknode(0, &operands);

    if (inode)
        service_register(&service, inode, "sysinfo");

}

