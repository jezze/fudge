#include <fudge.h>
#include <format/cpio.h>
#include "resource.h"
#include "binary.h"
#include "task.h"
#include "service.h"

static struct service_protocol protocol;

static struct cpio_header *getheader(struct service_backend *backend, unsigned int id)
{

    struct cpio_header *header = (struct cpio_header *)backend->map(id, sizeof (struct cpio_header));

    return (cpio_validate(header)) ? header : 0;

}

static char *getname(struct cpio_header *header)
{

    return (char *)(header + 1);

}

static unsigned int parent(struct service_backend *backend, struct cpio_header *header, unsigned int id)
{

    struct cpio_header *eheader;
    char dname[1024];
    unsigned int length = ascii_dname(dname, 1024, getname(header), header->namesize - 1, '/');
    unsigned int current = id;

    do
    {

        eheader = getheader(backend, current);

        if (!eheader)
            break;

        if ((eheader->mode & 0xF000) != 0x4000)
            continue;

        if (eheader->namesize == length + 1)
            return current;

    } while ((current = cpio_next(eheader, current)));

    return id;

}

static unsigned int child(struct service_backend *backend, struct cpio_header *header, unsigned int id, char *path, unsigned int length)
{

    struct cpio_header *eheader;
    unsigned int current = 0;

    do
    {

        if (current == id)
            break;

        eheader = getheader(backend, current);

        if (!eheader)
            break;

        if (eheader->namesize != header->namesize + length + 1)
            continue;

        if (memory_match(getname(eheader) + header->namesize, path, length))
            return current;

    } while ((current = cpio_next(eheader, current)));

    return id;

}

static unsigned int protocol_match(struct service_backend *backend)
{

    return getheader(backend, 0) != 0;

}

static unsigned int protocol_root(struct service_backend *backend)
{

    struct cpio_header *header;
    unsigned int id = 0;
    unsigned int current = id;

    do
    {

        header = getheader(backend, current);

        if (!header)
            break;

        if ((header->mode & 0xF000) != 0x4000)
            continue;

        id = current;

    } while ((current = cpio_next(header, current)));

    return id;

}

static unsigned int protocol_parent(struct service_backend *backend, struct service_state *state, unsigned int id)
{

    struct cpio_header *header = getheader(backend, id);

    if (!header)
        return 0;

    return parent(backend, header, id);

}

static unsigned int protocol_child(struct service_backend *backend, struct service_state *state, unsigned int id, char *path, unsigned int length)
{

    struct cpio_header *header = getheader(backend, id);

    if (!header)
        return 0;

    return child(backend, header, id, path, length);

}

static unsigned int protocol_create(struct service_backend *backend, struct service_state *state, unsigned int id, char *name, unsigned int length)
{

    return 0;

}

static unsigned int protocol_destroy(struct service_backend *backend, struct service_state *state, unsigned int id, char *name, unsigned int length)
{

    return 0;

}

static unsigned int stepdirectory(struct service_backend *backend, unsigned int id, unsigned int current)
{

    struct cpio_header *eheader = getheader(backend, current);

    if (!eheader)
        return 0;

    while ((current = cpio_next(eheader, current)))
    {

        if (current == id)
            break;

        eheader = getheader(backend, current);

        if (!eheader)
            break;

        if (parent(backend, eheader, current) == id)
            return current;

    }

    return 0;

}

static unsigned int protocol_step(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int current)
{

    struct cpio_header *header = getheader(backend, id);

    if (!header)
        return 0;

    switch (header->mode & 0xF000)
    {

    case 0x4000:
        return stepdirectory(backend, id, (id == current) ? 0 : current);

    }

    return 0;

}

static unsigned int protocol_open(struct service_backend *backend, struct service_state *state, unsigned int id)
{

    return id;

}

static unsigned int protocol_close(struct service_backend *backend, struct service_state *state, unsigned int id)
{

    return id;

}

static unsigned int readfile(struct service_backend *backend, void *buffer, unsigned int count, unsigned int offset, unsigned int id, struct cpio_header *header)
{

    unsigned int s = cpio_filesize(header) - offset;
    unsigned int o = id + cpio_filedata(header) + offset;

    return backend->read(buffer, (count > s) ? s : count, o);

}

static unsigned int readdirectory(struct service_backend *backend, void *buffer, unsigned int count, unsigned int offset, unsigned int current, struct cpio_header *header)
{

    struct record record;
    struct cpio_header *eheader;

    if (!current)
        return 0;

    eheader = getheader(backend, current);

    if (!eheader)
        return 0;

    record.id = current;
    record.size = cpio_filesize(eheader);
    record.length = memory_read(record.name, RECORD_NAMESIZE, eheader + 1, eheader->namesize - 1, header->namesize);

    return memory_read(buffer, count, &record, sizeof (struct record), offset);

}

static unsigned int protocol_read(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset)
{

    struct cpio_header *header = getheader(backend, id);

    if (!header)
        return 0;

    switch (header->mode & 0xF000)
    {

    case 0x8000:
        return readfile(backend, buffer, count, offset, id, header);

    case 0x4000:
        return readdirectory(backend, buffer, count, offset, current, header);

    }

    return 0;

}

static unsigned int writefile(struct service_backend *backend, void *buffer, unsigned int count, unsigned int offset, unsigned int id, struct cpio_header *header)
{

    unsigned int s = cpio_filesize(header) - offset;
    unsigned int o = id + cpio_filedata(header) + offset;

    return backend->write(buffer, (count > s) ? s : count, o);

}

static unsigned int protocol_write(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset)
{

    struct cpio_header *header = getheader(backend, id);

    if (!header)
        return 0;

    switch (header->mode & 0xF000)
    {

    case 0x8000:
        return writefile(backend, buffer, count, offset, id, header);

    }

    return 0;

}

static unsigned int protocol_seek(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int offset)
{

    return offset;

}

static unsigned int protocol_map(struct service_backend *backend, struct service_state *state, unsigned int id)
{

    struct cpio_header *header = getheader(backend, id);

    if (!header)
        return 0;

    return backend->map(id + cpio_filedata(header), cpio_filesize(header));

}

void service_setupcpio(void)
{

    service_initprotocol(&protocol, 1000, protocol_match, protocol_root, protocol_parent, protocol_child, protocol_create, protocol_destroy, protocol_step, protocol_open, protocol_close, protocol_read, protocol_write, protocol_seek, protocol_map);
    resource_register(&protocol.resource);

}

