#include <fudge.h>
#include <kernel.h>
#include "cpio.h"

static struct node node;
static struct service service;
static unsigned int address;
static unsigned int limit;

static struct cpio_header *getheader(unsigned int id)
{

    struct cpio_header *header = (struct cpio_header *)id;

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

static unsigned int getlist(unsigned int id, unsigned int offset, unsigned int count, struct record *records)
{

    struct cpio_header *header = getheader(id);
    unsigned int cid = address;
    unsigned int n = 0;

    do
    {

        struct cpio_header *cheader = getheader(cid);

        if (!cheader)
            break;

        if (getparent(cid) == id)
        {

            if (offset > 0)
            {

                offset--;

            }

            else
            {

                struct record *record = &records[n];

                record->id = cid;
                record->size = cpio_filesize(cheader);
                record->type = RECORD_TYPE_NORMAL;
                record->length = buffer_read(record->name, RECORD_NAMESIZE, getname(cid), cheader->namesize - 1, header->namesize);

                if ((cheader->mode & 0xF000) == 0x4000)
                    record->type = RECORD_TYPE_DIRECTORY;

                if (++n >= count)
                    break;

            }

        }

    } while ((cid = getnext(cid)));

    return n;

}

static unsigned int list(unsigned int id, unsigned int offset, unsigned int count, struct record *record)
{

    struct cpio_header *header = getheader(id);

    if (header)
    {

        switch (header->mode & 0xF000)
        {

        case 0x4000:
            return getlist(id, offset, count, record);

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

    if (header)
    {

        switch (header->mode & 0xF000)
        {

        case 0x8000:
            return buffer_read(buffer, count, (void *)(id + cpio_filedata(header)), cpio_filesize(header), offset);

        }

    }

    return 0;

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

static unsigned int write(unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    struct cpio_header *header = getheader(id);

    if (header)
    {

        switch (header->mode & 0xF000)
        {

        case 0x8000:
            return buffer_write((void *)(id + cpio_filedata(header)), cpio_filesize(header), buffer, count, offset);

        }

    }

    return 0;

}

static unsigned int onmaprequest(unsigned int source, unsigned int count, void *data)
{

    struct event_maprequest *maprequest = data;
    struct {struct event_mapresponse mapresponse;} message;

    message.mapresponse.session = maprequest->session;
    message.mapresponse.address = map(maprequest->id);

    return kernel_place(&node, source, EVENT_MAPRESPONSE, sizeof (struct event_mapresponse), &message);

}

static unsigned int onwalkrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_walkrequest *walkrequest = data;
    struct {struct event_walkresponse walkresponse;} message;

    message.walkresponse.session = walkrequest->session;
    message.walkresponse.id = walk((walkrequest->parent) ? walkrequest->parent : getroot(), (char *)(walkrequest + 1), walkrequest->length);

    return kernel_place(&node, source, EVENT_WALKRESPONSE, sizeof (struct event_walkresponse), &message);

}

static unsigned int onstatrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_statrequest *statrequest = data;
    struct {struct event_statresponse statresponse; struct record record;} message;

    message.statresponse.session = statrequest->session;
    message.statresponse.nrecords = stat(statrequest->id, &message.record);

    return kernel_place(&node, source, EVENT_STATRESPONSE, sizeof (struct event_statresponse) + sizeof (struct record), &message);

}

static unsigned int onlistrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_listrequest *listrequest = data;
    struct {struct event_listresponse listresponse; struct record records[8];} message;

    message.listresponse.session = listrequest->session;
    message.listresponse.nrecords = list(listrequest->id, listrequest->offset, (listrequest->nrecords > 8) ? 8 : listrequest->nrecords, message.records);

    return kernel_place(&node, source, EVENT_LISTRESPONSE, sizeof (struct event_listresponse) + sizeof (struct record) * message.listresponse.nrecords, &message);

}

static unsigned int onreadrequest(unsigned int source, unsigned int count, void *data)
{

    struct event_readrequest *readrequest = data;
    struct {struct event_readresponse readresponse; char data[64];} message;

    message.readresponse.session = readrequest->session;
    message.readresponse.count = read(readrequest->id, message.data, (readrequest->count > 64) ? 64 : readrequest->count, readrequest->offset);

    return kernel_place(&node, source, EVENT_READRESPONSE, sizeof (struct event_readresponse) + message.readresponse.count, &message);

}

static unsigned int onwriterequest(unsigned int source, unsigned int count, void *data)
{

    struct event_writerequest *writerequest = data;
    struct {struct event_writeresponse writeresponse;} message;

    message.writeresponse.session = writerequest->session;
    message.writeresponse.count = write(writerequest->id, writerequest + 1, writerequest->count, writerequest->offset);

    return kernel_place(&node, source, EVENT_WRITERESPONSE, sizeof (struct event_writeresponse), &message);

}

static struct node *service_match(unsigned int count, char *name)
{

    return &node;

}

static unsigned int place(struct node *source, struct node *target, unsigned int ichannel, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_MAPREQUEST:
        return onmaprequest(source->ichannel, count, data);

    case EVENT_WALKREQUEST:
        return onwalkrequest(source->ichannel, count, data);

    case EVENT_STATREQUEST:
        return onstatrequest(source->ichannel, count, data);

    case EVENT_LISTREQUEST:
        return onlistrequest(source->ichannel, count, data);

    case EVENT_READREQUEST:
        return onreadrequest(source->ichannel, count, data);

    case EVENT_WRITEREQUEST:
        return onwriterequest(source->ichannel, count, data);

    }

    return 0;

}

void cpio_setup(unsigned int addr, unsigned int lim)
{

    address = addr;
    limit = lim;

    node_init(&node, 0, place);
    service_init(&service, "initrd", service_match);
    service_register(&service);
    kernel_announce(&node);

}

