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
        {

            unsigned char pname[1024];

            if (!readname(backend, &eheader, id, 1024, pname))
                break;

            if (memory_match(name, pname, length))
                return id;

        }

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

static unsigned int protocol_parent(struct service_state *state)
{

    struct cpio_header header;

    if (!readheader(state->backend, &header, state->id))
        return 0;

    return parent(state->backend, &header, state->id);

}

static unsigned int protocol_child(struct service_state *state, unsigned int count, char *path)
{

    struct cpio_header header;
    unsigned char pname[1024];
    unsigned int cid = 0;
    unsigned int length;

    if (!count)
        return state->id;

    if (!readheader(state->backend, &header, state->id))
        return 0;

    if (!readname(state->backend, &header, state->id, 1024, pname))
        return 0;

    length = header.namesize;

    if (path[count - 1] == '/')
        count--;

    do
    {

        unsigned char cname[1024];

        if (cid == state->id)
            break;

        if (!readheader(state->backend, &header, cid))
            break;

        if (header.namesize - length != count + 1)
            continue;

        if (!readname(state->backend, &header, cid, 1024, cname))
            break;

        if (!memory_match(cname, pname, length - 1))
            continue;

        if (memory_match(cname + length, path, count))
            return cid;

    } while ((cid = cpio_next(&header, cid)));

    return 0;

}

static unsigned int scan(struct service_state *state, unsigned int index)
{

    struct cpio_header eheader;

    if (!readheader(state->backend, &eheader, index))
        return 0;

    while ((index = cpio_next(&eheader, index)))
    {

        if (index == state->id)
            break;

        if (!readheader(state->backend, &eheader, index))
            break;

        if (parent(state->backend, &eheader, index) == state->id)
            return index;

    }

    return 0;

}

static unsigned int seek(struct service_state *state, struct cpio_header *header, unsigned int offset)
{

    switch (header->mode & 0xF000)
    {

    case 0x4000:
        return scan(state, state->offset);

    case 0x8000:
        return offset;

    }

    return 0;

}

static unsigned int protocol_create(struct service_state *state, unsigned int count, char *name)
{

    return 0;

}

static unsigned int protocol_destroy(struct service_state *state, unsigned int count, char *name)
{

    return 0;

}

static unsigned int protocol_open(struct service_state *state)
{

    struct cpio_header header;

    if (!readheader(state->backend, &header, state->id))
        return 0;

    state->offset = seek(state, &header, 0);

    return state->id;

}

static unsigned int protocol_close(struct service_state *state)
{

    return state->id;

}

static unsigned int readnormal(struct service_state *state, unsigned int count, void *buffer, struct cpio_header *header)
{

    unsigned int s = cpio_filesize(header) - state->offset;
    unsigned int o = cpio_filedata(header, state->id) + state->offset;

    return state->backend->read(o, (count > s) ? s : count, buffer);

}

static unsigned int readdirectory(struct service_state *state, unsigned int count, void *buffer, struct cpio_header *header)
{

    struct record *record = buffer;
    struct cpio_header eheader;
    unsigned char name[1024];

    if (!state->offset)
        return 0;

    if (!readheader(state->backend, &eheader, state->offset))
        return 0;

    if (!readname(state->backend, &eheader, state->offset, 1024, name))
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

static unsigned int protocol_read(struct service_state *state, unsigned int count, void *buffer)
{

    struct cpio_header header;

    if (!readheader(state->backend, &header, state->id))
        return 0;

    switch (header.mode & 0xF000)
    {

    case 0x8000:
        count = readnormal(state, count, buffer, &header);

        break;

    case 0x4000:
        count = readdirectory(state, count, buffer, &header);

        break;

    default:
        count = 0;

        break;

    }

    state->offset = seek(state, &header, state->offset + count);

    return count;

}

static unsigned int writenormal(struct service_state *state, unsigned int count, void *buffer, struct cpio_header *header)
{

    unsigned int s = cpio_filesize(header) - state->offset;
    unsigned int o = cpio_filedata(header, state->id) + state->offset;

    return state->backend->write(o, (count > s) ? s : count, buffer);

}

static unsigned int protocol_write(struct service_state *state, unsigned int count, void *buffer)
{

    struct cpio_header header;

    if (!readheader(state->backend, &header, state->id))
        return 0;

    switch (header.mode & 0xF000)
    {

    case 0x8000:
        count = writenormal(state, count, buffer, &header);

        break;

    default:
        count = 0;

        break;

    }

    state->offset = seek(state, &header, state->offset + count);

    return count;

}

static unsigned int protocol_seek(struct service_state *state, unsigned int offset)
{

    struct cpio_header header;

    if (!readheader(state->backend, &header, state->id))
        return 0;

    return state->offset = seek(state, &header, offset);

}

static unsigned long protocol_map(struct service_state *state, struct binary_node *node)
{

    /* TEMPORARY FIX */
    struct cpio_header header;

    if (!readheader(state->backend, &header, state->id))
        return 0;

    return node->physical = state->backend->getphysical() + cpio_filedata(&header, state->id);

}

void service_setupcpio(void)
{

    service_initprotocol(&protocol, protocol_match, protocol_root, protocol_parent, protocol_child, protocol_create, protocol_destroy, protocol_open, protocol_close, protocol_read, protocol_write, protocol_seek, protocol_map);
    resource_register(&protocol.resource);

}

