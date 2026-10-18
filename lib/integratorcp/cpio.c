#include <fudge.h>
#include <kernel.h>
#include <disk.h>

static struct node_operands operands;
static struct service service;
static unsigned int address;
static unsigned int inode;

static struct cpio_header *getheader(unsigned int id)
{

    struct cpio_header *header = (struct cpio_header *)(unsigned long)id;

    return (cpio_validate(header)) ? header : 0;

}

static unsigned int getnext(unsigned int id)
{

    struct cpio_header *header = getheader(id);

    return (header) ? id + cpio_next(header) : 0;

}

/* an id is the address of a header, so one from a request is only trusted if walking the archive reaches it */
static unsigned int isheader(unsigned int id)
{

    unsigned int current = address;

    do
    {

        if (current == id)
            return 1;

    } while ((current = getnext(current)));

    return 0;

}

static unsigned int getroot(void)
{

    unsigned int current = address;
    unsigned int id = 0;

    do
    {

        struct cpio_header *cheader = getheader(current);

        if (!cheader)
            return id;

        if ((cpio_mode(cheader) & 0xF000) != 0x4000)
            continue;

        id = current;

    } while ((current = getnext(current)));

    return 0;

}

static unsigned int getparent(unsigned int id)
{

    struct cpio_header *header = getheader(id);
    unsigned int length = buffer_lastbyte(header + 1, cpio_namesize(header) - 1, '/');
    unsigned int current = id;

    do
    {

        struct cpio_header *cheader = getheader(current);

        if (!cheader)
            return 0;

        if ((cpio_mode(cheader) & 0xF000) != 0x4000)
            continue;

        if (cpio_namesize(cheader) == length)
            return current;

    } while ((current = getnext(current)));

    return 0;

}

static unsigned int getchild(unsigned int id, char *path, unsigned int length)
{

    struct cpio_header *header = getheader(id);
    unsigned int current = address;

    do
    {

        struct cpio_header *cheader = getheader(current);

        if (!cheader)
            return 0;

        if (cpio_namesize(cheader) != cpio_namesize(header) + length + 1)
            continue;

        if (buffer_match((char *)(cheader + 1) + cpio_namesize(header), path, length))
            return current;

    } while ((current = getnext(current)));

    return 0;

}

static unsigned int getrecord(unsigned int id, struct record *record)
{

    struct cpio_header *header = getheader(id);

    if (header)
    {

        unsigned int pid = getparent(id);

        if (pid)
        {

            struct cpio_header *pheader = getheader(pid);

            if (pheader)
            {

                switch (cpio_mode(header) & 0xF000)
                {

                case 0x4000:
                    record_init(record, id, RECORD_TYPE_DIRECTORY, cpio_filesize(header), getnext(id) - address, cpio_namesize(header) - cpio_namesize(pheader) - 1, (char *)(header + 1) + cpio_namesize(pheader));

                    break;

                case 0x8000:
                    record_init(record, id, RECORD_TYPE_NORMAL, cpio_filesize(header), getnext(id) - address, cpio_namesize(header) - cpio_namesize(pheader) - 1, (char *)(header + 1) + cpio_namesize(pheader));

                    break;

                }

                return sizeof (struct record);

            }

        }

    }

    return 0;

}

static unsigned int map(unsigned int id)
{

    struct cpio_header *header = getheader(id);

    return (header) ? id + cpio_filedata(header) : 0;

}

static unsigned long locate(unsigned int id)
{

    return (isheader(id)) ? map(id) : 0;

}

static unsigned int read(unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    struct cpio_header *header = getheader(id);

    return (header) ? buffer_read(buffer, count, (void *)((unsigned long)id + cpio_filedata(header)), cpio_filesize(header), offset) : 0;

}

static unsigned int readrecords(unsigned int id, struct record *records, unsigned int count, unsigned int offset)
{

    unsigned int current = (offset == 1) ? address : address + offset;
    unsigned int i = 0;
    unsigned int n = 0;

    if (!offset)
    {

        unsigned int parent = (id == getroot()) ? id : getparent(id);

        if (2 * sizeof (struct record) >= count)
            return 0;

        record_init(&records[0], id, RECORD_TYPE_DIRECTORY, 0, 1, 1, ".");
        record_init(&records[1], parent, RECORD_TYPE_DIRECTORY, 0, 1, 2, "..");

        i = 2;
        n = 2 * sizeof (struct record);

    }

    do
    {

        struct cpio_header *cheader = getheader(current);

        if (!cheader)
            break;

        if (getparent(current) == id)
        {

            if (n + sizeof (struct record) < count)
            {

                struct record *record = &records[i];

                n += getrecord(current, record);
                i += 1;

            }

        }

    } while ((current = getnext(current)));

    return n;

}

static unsigned int stat(unsigned int id, struct record *record)
{

    return getrecord(id, record);

}

static unsigned int walk(unsigned int id, char *path, unsigned int length)
{

    unsigned int offset = 0;

    while (offset < length)
    {

        char *cp = path + offset;
        unsigned int cl = buffer_findbyte(cp, length - offset, '/');

        if (cl == 0)
        {

            id = getroot();

        }

        else if (cl == 2 && cp[0] == '.' && cp[1] == '.')
        {

            if (id != getroot())
                id = getparent(id);

        }

        else if (cl != 1 || cp[0] != '.')
        {

            id = getchild(id, cp, cl);

        }

        if (!id)
            return 0;

        offset += cl + 1;

    }

    return id;

}

static unsigned int write(unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    struct cpio_header *header = getheader(id);

    return (header) ? buffer_write((void *)((unsigned long)id + cpio_filedata(header)), cpio_filesize(header), buffer, count, offset) : 0;

}

static unsigned int onmaprequest(unsigned int source, unsigned int count, void *data)
{

    struct event_maprequest *request = data;
    struct event_mapresponse response;

    response.address = (isheader(request->id)) ? map(request->id) : 0;

    return kernel_place(inode, source, EVENT_MAPRESPONSE, sizeof (struct event_mapresponse), &response);

}

static unsigned int onreadrequest(unsigned int source, unsigned int count, void *data)
{

    unsigned char buffer[MESSAGE_SIZE];
    struct event_readrequest *request = data;
    struct event_readresponse *response = (struct event_readresponse *)buffer;
    struct cpio_header *header = (isheader(request->id)) ? getheader(request->id) : 0;

    if (request->count >= MESSAGE_SIZE - sizeof (struct event_readresponse))
        request->count = MESSAGE_SIZE - sizeof (struct event_readresponse);

    response->count = 0;

    if (header)
    {

        switch (cpio_mode(header) & 0xF000)
        {

        case 0x4000:
            response->count = readrecords(request->id, (struct record *)(response + 1), request->count, request->offset);

            break;

        case 0x8000:
            response->count = read(request->id, response + 1, request->count, request->offset);

            break;

        default:
            response->count = 0;

            break;

        }

    }

    return kernel_place(inode, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + response->count, buffer);

}

static unsigned int onstatrequest(unsigned int source, unsigned int count, void *data)
{

    unsigned char buffer[MESSAGE_SIZE];
    struct event_statrequest *request = data;
    struct event_statresponse *response = (struct event_statresponse *)buffer;

    response->count = (isheader(request->id)) ? stat(request->id, (struct record *)(response + 1)) : 0;

    return kernel_place(inode, source, EVENT_STATRESPONSE, sizeof (struct event_statresponse) + response->count, response);

}

static unsigned int onwalkrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_walkrequest *request = data;
    struct event_walkresponse response;

    if (!request->parent)
        response.id = walk(getroot(), (char *)(request + 1), request->length);
    else if (isheader(request->parent))
        response.id = walk(request->parent, (char *)(request + 1), request->length);
    else
        response.id = 0;

    return kernel_place(inode, source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

}

static unsigned int onwriterequest(unsigned int source, unsigned int count, void *data)
{

    struct event_writerequest *request = data;
    struct event_writeresponse response;
    struct cpio_header *header = (isheader(request->id)) ? getheader(request->id) : 0;

    response.count = 0;

    if (header)
    {

        switch (cpio_mode(header) & 0xF000)
        {

        case 0x8000:
            response.count = write(request->id, request + 1, request->count, request->offset);

            break;

        default:
            response.count = 0;

            break;

        }

    }

    return kernel_place(inode, source, EVENT_WRITERESPONSE, sizeof (struct event_writeresponse), &response);

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_MAPREQUEST:
        return onmaprequest(source, count, data);

    case EVENT_READREQUEST:
        return onreadrequest(source, count, data);

    case EVENT_STATREQUEST:
        return onstatrequest(source, count, data);

    case EVENT_WALKREQUEST:
        return onwalkrequest(source, count, data);

    case EVENT_WRITEREQUEST:
        return onwriterequest(source, count, data);

    }

    return MESSAGE_FAILED;

}

void cpio_setup(unsigned int addr)
{

    node_operands_init(&operands, 0, operands_place, locate);
    service_init(&service);

    address = addr;
    inode = pool_picknode(0, &operands);

    if (inode)
        service_register(&service, inode, "initrd");

}


unsigned long cpio_find(char *path)
{

    unsigned int id = walk(getroot(), path, cstring_length(path));

    return (id) ? locate(id) : 0;

}
