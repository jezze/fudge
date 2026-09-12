#include <fudge.h>
#include <kernel.h>
#include <disk.h>

static struct node_operands operands;
static unsigned int address;
static unsigned int limit;
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

static unsigned int getroot(void)
{

    unsigned int current = address;
    unsigned int id = 0;

    do
    {

        struct cpio_header *cheader = getheader(current);

        if (!cheader)
            return id;

        if ((cheader->mode & 0xF000) != 0x4000)
            continue;

        id = current;

    } while ((current = getnext(current)));

    return 0;

}

static unsigned int getparent(unsigned int id)
{

    struct cpio_header *header = getheader(id);
    unsigned int length = buffer_lastbyte(header + 1, header->namesize - 1, '/');
    unsigned int current = id;

    do
    {

        struct cpio_header *cheader = getheader(current);

        if (!cheader)
            return 0;

        if ((cheader->mode & 0xF000) != 0x4000)
            continue;

        if (cheader->namesize == length)
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

        if (cheader->namesize != header->namesize + length + 1)
            continue;

        if (buffer_match((char *)(cheader + 1) + header->namesize, path, length))
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

                record->id = id;
                record->size = cpio_filesize(header);
                record->offset = getnext(id) - address;
                record->length = buffer_read(record->name, RECORD_NAMESIZE, header + 1, header->namesize - 1, pheader->namesize);

                switch (header->mode & 0xF000)
                {

                case 0x4000:
                    record->type = RECORD_TYPE_DIRECTORY;

                    break;

                case 0x8000:
                    record->type = RECORD_TYPE_NORMAL;

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

static unsigned int read(unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    struct cpio_header *header = getheader(id);

    return (header) ? buffer_read(buffer, count, (void *)((unsigned long)id + cpio_filedata(header)), cpio_filesize(header), offset) : 0;

}

static unsigned int readrecords(unsigned int id, struct record *records, unsigned int count, unsigned int offset)
{

    unsigned int current = address + offset;
    unsigned int i = 0;
    unsigned int n = 0;

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

    unsigned int offset = buffer_firstbyte(path, length, ':');

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

        else
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

    response.address = map(request->id);

    return kernel_place(inode, source, EVENT_MAPRESPONSE, sizeof (struct event_mapresponse), &response);

}

static unsigned int onreadrequest(unsigned int source, unsigned int count, void *data)
{

    unsigned char buffer[MESSAGE_SIZE];
    struct event_readrequest *request = data;
    struct event_readresponse *response = (struct event_readresponse *)buffer;
    struct cpio_header *header = getheader(request->id);

    if (request->count >= MESSAGE_SIZE - sizeof (struct event_readresponse))
        request->count = MESSAGE_SIZE - sizeof (struct event_readresponse);

    if (header)
    {

        switch (header->mode & 0xF000)
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

    struct event_statrequest *request = data;
    struct record record;

    return kernel_place(inode, source, EVENT_STATRESPONSE, stat(request->id, &record) * sizeof (struct record), &record);

}

static unsigned int onwalkrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_walkrequest *request = data;
    struct event_walkresponse response;

    response.id = walk((request->parent) ? request->parent : getroot(), (char *)(request + 1), request->length);

    return kernel_place(inode, source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

}

static unsigned int onwriterequest(unsigned int source, unsigned int count, void *data)
{

    struct event_writerequest *request = data;
    struct event_writeresponse response;
    struct cpio_header *header = getheader(request->id);

    if (header)
    {

        switch (header->mode & 0xF000)
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

static unsigned int operands_place(struct resource *resource, unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
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

void cpio_setup(unsigned int addr, unsigned int lim)
{

    address = addr;
    limit = lim;
    inode = pool_picknode();

    node_operands_init(&operands, 0, operands_place);

    if (inode)
    {

        struct node *node = pool_getnode(inode);

        node_reset(node, "initrd", 0, &operands);

    }

}

