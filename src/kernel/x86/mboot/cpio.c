#include <fudge.h>
#include <kernel.h>
#include "cpio.h"

static struct service service;
static unsigned int address;
static unsigned int limit;

static struct cpio_header *getheader(unsigned int id)
{

    struct cpio_header *header = (struct cpio_header *)id;

    return (cpio_validate(header)) ? header : 0;

}

unsigned int getnext(unsigned int id)
{

    struct cpio_header *header = getheader(id);

    return (header) ? id + cpio_next(header) : 0;

}

static char *getname(unsigned int id)
{

    struct cpio_header *header = getheader(id);

    return (header) ? (char *)(header + 1) : 0;

}

static unsigned int parent(struct cpio_header *header, unsigned int id)
{

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

static unsigned int child(struct cpio_header *header, unsigned int id, char *path, unsigned int length)
{

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

static unsigned int service_root(void)
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

static unsigned int service_parent(unsigned int id)
{

    struct cpio_header *header = getheader(id);

    return (header) ? parent(header, id) : 0;

}

static unsigned int service_child(unsigned int id, char *path, unsigned int length)
{

    struct cpio_header *header = getheader(id);

    return (header) ? child(header, id, path, length) : 0;

}

static unsigned int service_create(unsigned int id, char *name, unsigned int length)
{

    return 0;

}

static unsigned int service_destroy(unsigned int id)
{

    return 0;

}

static unsigned int service_stat(unsigned int id, struct record *record)
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

static unsigned int getlist(unsigned int id, unsigned int cid, unsigned int count, struct record *records)
{

    struct cpio_header *header = getheader(id);
    unsigned int n = 0;

    cid = (cid) ? getnext(cid) : address;

    do
    {

        struct cpio_header *cheader = getheader(cid);

        if (!cheader)
            break;

        if (cid == id)
            break;

        if (parent(cheader, cid) == id)
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

    } while ((cid = getnext(cid)));

    return n;

}

static unsigned int service_list(unsigned int id, unsigned int cid, unsigned int count, struct record *record)
{

    struct cpio_header *header = getheader(id);

    if (header)
    {

        switch (header->mode & 0xF000)
        {

        case 0x4000:
            return getlist(id, cid, count, record);

        }

    }

    return 0;

}

static unsigned int service_read(unsigned int id, void *buffer, unsigned int count, unsigned int offset)
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

static unsigned int service_write(unsigned int id, void *buffer, unsigned int count, unsigned int offset)
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

static unsigned int service_map(unsigned int id, unsigned int address)
{

    struct cpio_header *header = getheader(id);

    return (header) ? id + cpio_filedata(header) : 0;

}

static unsigned int service_link(unsigned int id, unsigned int target, unsigned int source)
{

    return 0;

}

static unsigned int service_unlink(unsigned int id, unsigned int target)
{

    return 0;

}

static unsigned int service_notify(unsigned int id, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    return 0;

}

void cpio_setup(unsigned int addr, unsigned int lim)
{

    address = addr;
    limit = lim;

    service_init(&service, "initrd", service_root, service_parent, service_child, service_create, service_destroy, service_stat, service_list, service_read, service_write, service_map, service_link, service_unlink, service_notify);
    resource_register(&service.resource);

}

