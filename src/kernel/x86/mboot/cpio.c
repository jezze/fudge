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

static char *getname(unsigned int id)
{

    struct cpio_header *header = getheader(id);

    return (header) ? (char *)(header + 1) : 0;

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
    unsigned int length = buffer_lastbyte(getname(id), header->namesize - 1, '/');
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

        if (buffer_match(getname(current) + header->namesize, path, length))
            return current;

    } while ((current = getnext(current)));

    return 0;

}

static unsigned int getlist(unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct cpio_header *header = getheader(id);
    unsigned int cid = address;
    unsigned int i = 0;
    unsigned int n = 0;
    struct record *records = (struct record *)buffer;

    do
    {

        struct cpio_header *cheader = getheader(cid);

        if (!cheader)
            break;

        if (getparent(cid) == id)
        {

            if (offset > 0)
            {

                offset -= sizeof (struct record);

            }

            else
            {

                struct record *record = &records[i];

                record->id = cid;
                record->size = cpio_filesize(cheader);
                record->type = RECORD_TYPE_NORMAL;
                record->length = buffer_read(record->name, RECORD_NAMESIZE, getname(cid), cheader->namesize - 1, header->namesize);

                if ((cheader->mode & 0xF000) == 0x4000)
                    record->type = RECORD_TYPE_DIRECTORY;

                n += sizeof (struct record);
                i += 1;

                if (n >= count)
                    break;

            }

        }

    } while ((cid = getnext(cid)));

    return n;

}

static unsigned int map(unsigned int id)
{

    struct cpio_header *header = getheader(id);

    return (header) ? id + cpio_filedata(header) : 0;

}

static unsigned int stat(unsigned int id, struct record *record)
{

    struct cpio_header *header = getheader(id);

    if (header)
    {

        record->id = id;
        record->size = cpio_filesize(header);
        record->type = RECORD_TYPE_NORMAL;
        record->length = buffer_read(record->name, RECORD_NAMESIZE, getname(id), header->namesize - 1, header->namesize);

        if ((header->mode & 0xF000) == 0x4000)
            record->type = RECORD_TYPE_DIRECTORY;

        return 1;

    }

    return 0;

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

static unsigned int onmaprequest(unsigned int source, unsigned int count, void *data)
{

    struct event_maprequest *request = data;
    struct event_mapresponse response;

    response.address = map(request->id);

    return kernel_place(inode, source, EVENT_MAPRESPONSE, sizeof (struct event_mapresponse), &response);

}

static unsigned int onwalkrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_walkrequest *request = data;
    struct event_walkresponse response;

    response.id = walk((request->parent) ? request->parent : getroot(), (char *)(request + 1), request->length);

    return kernel_place(inode, source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &response);

}

static unsigned int onstatrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_statrequest *request = data;
    struct record record;

    return kernel_place(inode, source, EVENT_STATRESPONSE, stat(request->id, &record) * sizeof (struct record), &record);

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
            response->count = getlist(request->id, request->offset, request->count, response + 1);

            break;

        case 0x8000:
            response->count = buffer_read(response + 1, request->count, (void *)((unsigned long)request->id + cpio_filedata(header)), cpio_filesize(header), request->offset);

            break;

        default:
            response->count = 0;

            break;

        }

    }

    return kernel_place(inode, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + response->count, buffer);

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
            response.count = buffer_write((void *)((unsigned long)request->id + cpio_filedata(header)), cpio_filesize(header), request + 1, request->count, request->offset);

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

    case EVENT_WALKREQUEST:
        return onwalkrequest(source, count, data);

    case EVENT_STATREQUEST:
        return onstatrequest(source, count, data);

    case EVENT_READREQUEST:
        return onreadrequest(source, count, data);

    case EVENT_WRITEREQUEST:
        return onwriterequest(source, count, data);

    }

    return 0;

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

