#include <fudge.h>
#include <format/cpio.h>
#include "resource.h"
#include "binary.h"
#include "service.h"

static struct service_protocol protocol;

static unsigned int decode(unsigned int id)
{

    return id - 4096;

}

static unsigned int encode(unsigned int address)
{

    return address + 4096;

}

static unsigned int readheader(struct service_backend *backend, struct cpio_header *header, unsigned int address)
{

    unsigned int count = backend->read(address, sizeof (struct cpio_header), header);

    return (count == sizeof (struct cpio_header)) ? cpio_validate(header) : 0;

}

static unsigned int readname(struct service_backend *backend, struct cpio_header *header, unsigned int address, unsigned int count, void *buffer)
{

    return (header->namesize <= count) ? backend->read(address + sizeof (struct cpio_header), header->namesize, buffer) : 0;

}

static unsigned int protocol_match(struct service_backend *backend)
{

    struct cpio_header header;

    return readheader(backend, &header, 0);

}

static unsigned int protocol_root(struct service_backend *backend)
{

    struct cpio_header header;
    unsigned int address = 0;
    unsigned int last = address;

    do
    {

        if (!readheader(backend, &header, address))
            break;

        if ((header.mode & 0xF000) != 0x4000)
            continue;

        last = address;

    } while ((address = cpio_next(&header, address)));

    return encode(last);

}

static unsigned int protocol_parent(struct service_backend *backend, unsigned int id)
{

    struct cpio_header header;
    unsigned char name[1024];
    unsigned int address = decode(id);
    unsigned int length;

    if (id == protocol_root(backend))
        return id;

    if (!readheader(backend, &header, address))
        return 0;

    if (!readname(backend, &header, address, 1024, name))
        return 0;

    length = header.namesize - 1;

    while (--length && name[length] != '/');

    do
    {

        if (!readheader(backend, &header, address))
            break;

        if ((header.mode & 0xF000) != 0x4000)
            continue;

        if (header.namesize == length + 1)
        {

            unsigned char pname[1024];

            if (!readname(backend, &header, address, 1024, pname))
                break;

            if (memory_match(name, pname, length))
                return encode(address);

        }

    } while ((address = cpio_next(&header, address)));

    return 0;

}

static unsigned int protocol_child(struct service_backend *backend, unsigned int id, unsigned int count, char *path)
{

    struct cpio_header header;
    unsigned char pname[1024];
    unsigned int address = decode(id);
    unsigned int length;

    if (!count)
        return id;

    if (!readheader(backend, &header, address))
        return 0;

    if (!readname(backend, &header, address, 1024, pname))
        return 0;

    length = header.namesize;
    address = 0;

    if (path[count - 1] == '/')
        count--;

    do
    {

        unsigned int cid = encode(address);
        unsigned char cname[1024];

        if (cid == id)
            break;

        if (!readheader(backend, &header, address))
            break;

        if (header.namesize - length != count + 1)
            continue;

        if (!readname(backend, &header, address, 1024, cname))
            break;

        if (!memory_match(cname, pname, length - 1))
            continue;

        if (memory_match(cname + length, path, count))
            return cid;

    } while ((address = cpio_next(&header, address)));

    return 0;

}

static unsigned int protocol_create(struct service_backend *backend, unsigned int id, unsigned int count, char *name)
{

    return 0;

}

static unsigned int protocol_destroy(struct service_backend *backend, unsigned int id, unsigned int count, char *name)
{

    return 0;

}

static unsigned int protocol_open(struct service_backend *backend, struct list_item *link, struct service_state *state)
{

    state->offset = 0;

    return state->id;

}

static unsigned int protocol_close(struct service_backend *backend, struct list_item *link, struct service_state *state)
{

    state->offset = 0;

    return state->id;

}

static unsigned int readnormal(struct service_backend *backend, unsigned int address, struct service_state *state, unsigned int count, void *buffer, struct cpio_header *header)
{

    unsigned int s = cpio_filesize(header) - state->offset;
    unsigned int o = cpio_filedata(header, address) + state->offset;

    return backend->read(o, (count > s) ? s : count, buffer);

}

static unsigned int readdirectory(struct service_backend *backend, unsigned int address, struct service_state *state, unsigned int count, void *buffer, struct cpio_header *header)
{

    struct record *record = buffer;
    unsigned int length = header->namesize;
    unsigned char name[1024];

    address = (state->offset) ? decode(state->offset) : decode(protocol_parent(backend, encode(address)));

    if (!readheader(backend, header, address))
        return 0;

    if (!readname(backend, header, address, 1024, name))
        return 0;

    record->id = encode(address);
    record->size = cpio_filesize(header);
    record->length = memory_read(record->name, RECORD_NAMESIZE, name, header->namesize - 1, length);

    switch (header->mode & 0xF000)
    {

    case 0x4000:
        record->length += memory_write(record->name, RECORD_NAMESIZE, "/", 1, record->length);

        break;

    }

    return sizeof (struct record);

}

static unsigned int protocol_read(struct service_backend *backend, struct list_item *link, struct service_state *state, unsigned int count, void *buffer)
{

    struct cpio_header header;
    unsigned int address = decode(state->id);

    if (!readheader(backend, &header, address))
        return 0;

    switch (header.mode & 0xF000)
    {

    case 0x8000:
        count = readnormal(backend, address, state, count, buffer, &header);

        break;

    case 0x4000:
        count = readdirectory(backend, address, state, count, buffer, &header);

        break;

    default:
        count = 0;

        break;

    }

    state->offset += count;

    return count;

}

static unsigned int writenormal(struct service_backend *backend, unsigned int address, struct service_state *state, unsigned int count, void *buffer, struct cpio_header *header)
{

    unsigned int s = cpio_filesize(header) - state->offset;
    unsigned int o = cpio_filedata(header, address) + state->offset;

    return backend->write(o, (count > s) ? s : count, buffer);

}

static unsigned int protocol_write(struct service_backend *backend, struct list_item *link, struct service_state *state, unsigned int count, void *buffer)
{

    struct cpio_header header;
    unsigned int address = decode(state->id);

    if (!readheader(backend, &header, address))
        return 0;

    switch (header.mode & 0xF000)
    {

    case 0x8000:
        count = writenormal(backend, address, state, count, buffer, &header);

        break;

    default:
        count = 0;

        break;

    }

    state->offset += count;

    return count;

}

static unsigned int protocol_seek(struct service_backend *backend, struct service_state *state, unsigned int offset)
{

    return state->offset = offset;

}

static unsigned int protocol_scan(struct service_backend *backend, struct service_state *state, unsigned int index)
{

    struct cpio_header header;
    unsigned int address = (index) ? decode(index) : 0;

    if (!readheader(backend, &header, decode(state->id)))
        return 0;

    if ((header.mode & 0xF000) != 0x4000)
        return 0;

    if (!readheader(backend, &header, address))
        return 0;

    while ((address = cpio_next(&header, address)))
    {

        if (encode(address) == state->id)
            break;

        if (!readheader(backend, &header, address))
            break;

        if (protocol_parent(backend, encode(address)) == state->id)
            return encode(address);

    }

    return 0;

}

static unsigned long protocol_map(struct service_backend *backend, unsigned int id, struct binary_node *node)
{

    /* TEMPORARY FIX */
    struct cpio_header header;
    unsigned int address = decode(id);

    if (!readheader(backend, &header, address))
        return 0;

    return node->physical = backend->getphysical() + cpio_filedata(&header, address);

}

void service_setupcpio(void)
{

    service_initprotocol(&protocol, protocol_match, protocol_root, protocol_parent, protocol_child, protocol_create, protocol_destroy, protocol_open, protocol_close, protocol_read, protocol_write, protocol_seek, protocol_scan, protocol_map);
    resource_register(&protocol.resource);

}

