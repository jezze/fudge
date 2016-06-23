#include <fudge.h>
#include <format/cpio.h>
#include "resource.h"
#include "binary.h"
#include "service.h"

static struct service_protocol protocol;

static unsigned int readheader(struct service_backend *backend, struct cpio_header *header, unsigned int id)
{

    unsigned int count = backend->read(id, sizeof (struct cpio_header), header);

    return (count == sizeof (struct cpio_header)) ? cpio_validate(header) : 0;

}

static unsigned int readname(struct service_backend *backend, struct cpio_header *header, unsigned int id, unsigned int count, void *buffer)
{

    return (header->namesize <= count) ? backend->read(id + sizeof (struct cpio_header), header->namesize, buffer) : 0;

}

static unsigned int parent(struct service_backend *backend, struct cpio_header *header, unsigned int id)
{

    struct cpio_header eheader;
    unsigned char name[1024];
    unsigned int length;

    if (!readname(backend, header, id, 1024, name))
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

    if (!readname(backend, &header, state->id, 1024, name))
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

static unsigned int protocol_child(struct service_backend *backend, struct service_state *state, unsigned int count, char *path)
{

    struct cpio_header header;
    struct cpio_header eheader;
    unsigned char name[1024];
    unsigned int id = 0;

    if (!count)
        return 1;

    if (!readheader(backend, &header, state->id))
        return 0;

    if (!readname(backend, &header, state->id, 1024, name))
        return 0;

    if (path[count - 1] == '/')
        count--;

    do
    {

        unsigned char cname[1024];

        if (id == state->id)
            break;

        if (!readheader(backend, &eheader, id))
            break;

        if (eheader.namesize - header.namesize != count + 1)
            continue;

        if (!readname(backend, &eheader, id, 1024, cname))
            break;

        if (memory_match(cname + header.namesize, path, count))
        {

            state->id = id;

            return 1;

        }

    } while ((id = cpio_next(&eheader, id)));

    return 0;

}

static unsigned int scan(struct service_backend *backend, struct service_state *state, unsigned int index)
{

    struct cpio_header eheader;

    if (!readheader(backend, &eheader, index))
        return 0;

    while ((index = cpio_next(&eheader, index)))
    {

        if (index == state->id)
            break;

        if (!readheader(backend, &eheader, index))
            break;

        if (parent(backend, &eheader, index) == state->id)
            return index;

    }

    return 0;

}

static unsigned int seek(struct service_backend *backend, struct service_state *state, struct cpio_header *header, unsigned int offset)
{

    switch (header->mode & 0xF000)
    {

    case 0x4000:
        return scan(backend, state, state->offset);

    case 0x8000:
        return offset;

    }

    return 0;

}

static unsigned int protocol_create(struct service_backend *backend, struct service_state *state, unsigned int count, char *name)
{

    return 0;

}

static unsigned int protocol_destroy(struct service_backend *backend, struct service_state *state, unsigned int count, char *name)
{

    return 0;

}

static unsigned int protocol_open(struct service_backend *backend, struct service_state *state)
{

    struct cpio_header header;

    if (!readheader(backend, &header, state->id))
        return 0;

    state->offset = 0;
    state->offset = seek(backend, state, &header, 0);

    return state->id;

}

static unsigned int protocol_close(struct service_backend *backend, struct service_state *state)
{

    state->offset = 0;

    return state->id;

}

static unsigned int readnormal(struct service_backend *backend, struct service_state *state, unsigned int count, void *buffer, struct cpio_header *header)
{

    unsigned int s = cpio_filesize(header) - state->offset;
    unsigned int o = cpio_filedata(header, state->id) + state->offset;

    return backend->read(o, (count > s) ? s : count, buffer);

}

static unsigned int readdirectory(struct service_backend *backend, struct service_state *state, unsigned int count, void *buffer, struct cpio_header *header)
{

    struct record *record = buffer;
    struct cpio_header eheader;
    unsigned char name[1024];

    if (!state->offset)
        return 0;

    if (!readheader(backend, &eheader, state->offset))
        return 0;

    if (!readname(backend, &eheader, state->offset, 1024, name))
        return 0;

    record->id = state->offset;
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

static unsigned int protocol_read(struct service_backend *backend, struct service_state *state, unsigned int count, void *buffer)
{

    struct cpio_header header;

    if (!readheader(backend, &header, state->id))
        return 0;

    switch (header.mode & 0xF000)
    {

    case 0x8000:
        count = readnormal(backend, state, count, buffer, &header);

        break;

    case 0x4000:
        count = readdirectory(backend, state, count, buffer, &header);

        break;

    default:
        count = 0;

        break;

    }

    state->offset = seek(backend, state, &header, state->offset + count);

    return count;

}

static unsigned int writenormal(struct service_backend *backend, struct service_state *state, unsigned int count, void *buffer, struct cpio_header *header)
{

    unsigned int s = cpio_filesize(header) - state->offset;
    unsigned int o = cpio_filedata(header, state->id) + state->offset;

    return backend->write(o, (count > s) ? s : count, buffer);

}

static unsigned int protocol_write(struct service_backend *backend, struct service_state *state, unsigned int count, void *buffer)
{

    struct cpio_header header;

    if (!readheader(backend, &header, state->id))
        return 0;

    switch (header.mode & 0xF000)
    {

    case 0x8000:
        count = writenormal(backend, state, count, buffer, &header);

        break;

    default:
        count = 0;

        break;

    }

    state->offset = seek(backend, state, &header, state->offset + count);

    return count;

}

static unsigned int protocol_seek(struct service_backend *backend, struct service_state *state, unsigned int offset)
{

    struct cpio_header header;

    if (!readheader(backend, &header, state->id))
        return 0;

    return state->offset = seek(backend, state, &header, offset);

}

static unsigned long protocol_map(struct service_backend *backend, struct service_state *state, struct binary_node *node)
{

    /* TEMPORARY FIX */
    struct cpio_header header;

    if (!readheader(backend, &header, state->id))
        return 0;

    return node->physical = backend->getphysical() + cpio_filedata(&header, state->id);

}

void service_setupcpio(void)
{

    service_initprotocol(&protocol, protocol_match, protocol_root, protocol_parent, protocol_child, protocol_create, protocol_destroy, protocol_open, protocol_close, protocol_read, protocol_write, protocol_seek, protocol_map);
    resource_register(&protocol.resource);

}

