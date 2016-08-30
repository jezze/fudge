#include <fudge.h>
#include <format/cpio.h>
#include "resource.h"
#include "binary.h"
#include "service.h"

static struct service_protocol protocol;

static unsigned int readheader(struct service_backend *backend, struct cpio_header *header, unsigned int id)
{

    unsigned int count = backend->read(header, sizeof (struct cpio_header), id);

    return (count == sizeof (struct cpio_header)) ? cpio_validate(header) : 0;

}

static unsigned int readname(struct service_backend *backend, struct cpio_header *header, unsigned int id, void *buffer, unsigned int count)
{

    return (header->namesize <= count) ? backend->read(buffer, header->namesize, id + sizeof (struct cpio_header)) : 0;

}

static unsigned int parent(struct service_backend *backend, struct cpio_header *header, unsigned int id)
{

    struct cpio_header eheader;
    unsigned char name[1024];
    unsigned int length;

    if (!readname(backend, header, id, name, 1024))
        return 0;

    for (length = header->namesize - 1; length && name[length] != '/'; length--);

    do
    {

        if (!readheader(backend, &eheader, id))
            break;

        if ((eheader.mode & 0xF000) != 0x4000)
            continue;

        if (eheader.namesize == length + 1)
            return id;

    } while ((id = cpio_next(&eheader, id)));

    return 0;

}

static unsigned int next(struct service_backend *backend, unsigned int id, unsigned int index)
{

    struct cpio_header eheader;

    if (!readheader(backend, &eheader, index))
        return 0;

    while ((index = cpio_next(&eheader, index)))
    {

        if (index == id)
            break;

        if (!readheader(backend, &eheader, index))
            break;

        if (parent(backend, &eheader, index) == id)
            return index;

    }

    return 0;

}

static unsigned int protocol_match(struct service_backend *backend)
{

    struct cpio_header header;

    return readheader(backend, &header, 0);

}

static unsigned int protocol_root(struct service_backend *backend)
{

    struct cpio_header header;
    unsigned int id = 0;
    unsigned int last = id;

    do
    {

        if (!readheader(backend, &header, id))
            break;

        if ((header.mode & 0xF000) != 0x4000)
            continue;

        last = id;

    } while ((id = cpio_next(&header, id)));

    return last;

}

static unsigned int protocol_parent(struct service_backend *backend, struct service_state *state)
{

    struct cpio_header header;
    struct cpio_header eheader;
    unsigned char name[1024];
    unsigned int id = state->id;

    if (!readheader(backend, &header, state->id))
        return 0;

    if (!readname(backend, &header, state->id, name, 1024))
        return 0;

    while (name[header.namesize] != '/')
        header.namesize--;

    do
    {

        if (!readheader(backend, &eheader, id))
            break;

        if ((eheader.mode & 0xF000) != 0x4000)
            continue;

        if (eheader.namesize == header.namesize + 1)
        {

            state->id = id;

            return 1;

        }

    } while ((id = cpio_next(&eheader, id)));

    return 0;

}

static unsigned int protocol_child(struct service_backend *backend, struct service_state *state, char *path, unsigned int length)
{

    struct cpio_header header;
    struct cpio_header eheader;
    unsigned char name[1024];
    unsigned int id = 0;

    if (!length)
        return 1;

    if (!readheader(backend, &header, state->id))
        return 0;

    if (!readname(backend, &header, state->id, name, 1024))
        return 0;

    if (path[length - 1] == '/')
        length--;

    do
    {

        unsigned char cname[1024];

        if (id == state->id)
            break;

        if (!readheader(backend, &eheader, id))
            break;

        if (eheader.namesize - header.namesize != length + 1)
            continue;

        if (!readname(backend, &eheader, id, cname, 1024))
            break;

        if (memory_match(cname + header.namesize, path, length))
        {

            state->id = id;

            return 1;

        }

    } while ((id = cpio_next(&eheader, id)));

    return 0;

}

static unsigned int protocol_create(struct service_backend *backend, struct service_state *state, char *name, unsigned int length)
{

    return 0;

}

static unsigned int protocol_destroy(struct service_backend *backend, struct service_state *state, char *name, unsigned int length)
{

    return 0;

}

static unsigned int protocol_step(struct service_backend *backend, unsigned int id, unsigned int current)
{

    struct cpio_header header;

    if (!readheader(backend, &header, id))
        return 0;

    switch (header.mode & 0xF000)
    {

    case 0x4000:
        return next(backend, id, current);

    }

    return 0;

}

static unsigned int protocol_open(struct service_backend *backend, struct service_state *state)
{

    return state->id;

}

static unsigned int protocol_close(struct service_backend *backend, struct service_state *state)
{

    return state->id;

}

static unsigned int readnormal(struct service_backend *backend, struct service_state *state, void *buffer, unsigned int count, struct cpio_header *header)
{

    unsigned int s = cpio_filesize(header) - state->offset;
    unsigned int o = cpio_filedata(header, state->id) + state->offset;

    return backend->read(buffer, (count > s) ? s : count, o);

}

static unsigned int readdirectory(struct service_backend *backend, struct service_state *state, void *buffer, unsigned int count, struct cpio_header *header)
{

    struct record *record = buffer;
    struct cpio_header eheader;
    unsigned char name[1024];

    if (!state->current)
        return 0;

    if (!readheader(backend, &eheader, state->current))
        return 0;

    if (!readname(backend, &eheader, state->current, name, 1024))
        return 0;

    record->id = state->current;
    record->size = cpio_filesize(&eheader);
    record->length = memory_read(record->name, RECORD_NAMESIZE, name, eheader.namesize - 1, header->namesize);

    switch (eheader.mode & 0xF000)
    {

    case 0x4000:
        record->length += memory_write(record->name, RECORD_NAMESIZE, "/", 1, record->length);

        break;

    }

    return sizeof (struct record);

}

static unsigned int protocol_read(struct service_backend *backend, struct service_state *state, void *buffer, unsigned int count)
{

    struct cpio_header header;

    if (!readheader(backend, &header, state->id))
        return 0;

    switch (header.mode & 0xF000)
    {

    case 0x8000:
        return readnormal(backend, state, buffer, count, &header);

    case 0x4000:
        return readdirectory(backend, state, buffer, count, &header);

    }

    return 0;

}

static unsigned int writenormal(struct service_backend *backend, struct service_state *state, void *buffer, unsigned int count, struct cpio_header *header)
{

    unsigned int s = cpio_filesize(header) - state->offset;
    unsigned int o = cpio_filedata(header, state->id) + state->offset;

    return backend->write(buffer, (count > s) ? s : count, o);

}

static unsigned int protocol_write(struct service_backend *backend, struct service_state *state, void *buffer, unsigned int count)
{

    struct cpio_header header;

    if (!readheader(backend, &header, state->id))
        return 0;

    switch (header.mode & 0xF000)
    {

    case 0x8000:
        return writenormal(backend, state, buffer, count, &header);

    }

    return 0;

}

static unsigned int protocol_seek(struct service_backend *backend, unsigned int offset)
{

    return offset;

}

static unsigned long protocol_map(struct service_backend *backend, unsigned int id, struct binary_node *node)
{

    /* TEMPORARY FIX */
    struct cpio_header header;

    if (!readheader(backend, &header, id))
        return 0;

    return node->physical = backend->getphysical() + cpio_filedata(&header, id);

}

void service_setupcpio(void)
{

    service_initprotocol(&protocol, protocol_match, protocol_root, protocol_parent, protocol_child, protocol_create, protocol_destroy, protocol_step, protocol_open, protocol_close, protocol_read, protocol_write, protocol_seek, protocol_map);
    resource_register(&protocol.resource);

}

