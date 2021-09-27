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

    unsigned int length = buffer_findlastbyte(getname(id), header->namesize - 1, '/');
    unsigned int current = id;

    do
    {

        struct cpio_header *cheader = getheader(current);

        if (!cheader)
            return 0;

        if ((cheader->mode & 0xF000) != 0x4000)
            continue;

        if (cheader->namesize == length + 1)
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

static unsigned int stepdirectory(unsigned int id, unsigned int current)
{

    do
    {

        struct cpio_header *cheader = getheader(current);

        if (!cheader)
            return 0;

        if (current == id)
            return 0;

        if (parent(cheader, current) == id)
            return current;

    } while ((current = getnext(current)));

    return 0;

}

static unsigned int service_step(unsigned int id, unsigned int current)
{

    struct cpio_header *header = getheader(id);

    if (!header)
        return 0;

    switch (header->mode & 0xF000)
    {

    case 0x4000:
        return stepdirectory(id, (current) ? getnext(current) : address);

    }

    return 0;

}

static unsigned int readdirectory(void *buffer, unsigned int count, unsigned int offset, unsigned int current, struct cpio_header *header)
{

    struct cpio_header *cheader = getheader(current);

    if (cheader)
    {

        struct record record;

        record.id = current;
        record.size = cpio_filesize(cheader);
        record.length = buffer_read(record.name, RECORD_NAMESIZE, getname(current), cheader->namesize - 1, header->namesize);

        return buffer_read(buffer, count, &record, sizeof (struct record), offset);

    }

    return 0;

}

static unsigned int service_read(unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset)
{

    struct cpio_header *header = getheader(id);

    if (header)
    {

        switch (header->mode & 0xF000)
        {

        case 0x8000:
            return buffer_read(buffer, count, (void *)(id + cpio_filedata(header)), cpio_filesize(header), offset);

        case 0x4000:
            return readdirectory(buffer, count, offset, current, header);

        }

    }

    return 0;

}

static unsigned int service_write(unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset)
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

static unsigned int service_seek(unsigned int id, unsigned int offset)
{

    return offset;

}

static unsigned int service_map(unsigned int id)
{

    struct cpio_header *header = getheader(id);

    return (header) ? id + cpio_filedata(header) : 0;

}

static unsigned int service_link(unsigned int id, unsigned int source)
{

    return 0;

}

static unsigned int service_unlink(unsigned int id, unsigned int source)
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

    service_init(&service, "initrd", service_root, service_parent, service_child, service_create, service_destroy, service_step, service_read, service_write, service_seek, service_map, service_link, service_unlink, service_notify);
    resource_register(&service.resource);

}

