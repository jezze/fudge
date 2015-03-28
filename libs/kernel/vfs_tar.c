#include <fudge.h>
#include <tar/tar.h>
#include "resource.h"
#include "vfs.h"

static struct vfs_protocol protocol;

static unsigned int protocol_match(struct vfs_backend *backend)
{

    struct tar_header header;

    if (backend->read(backend, 0, TAR_BLOCK_SIZE, &header) < TAR_BLOCK_SIZE)
        return 0;

    return tar_validate(&header);

}

static unsigned int protocol_root(struct vfs_backend *backend)
{

    return TAR_BLOCK_SIZE;

}

static unsigned int decode(unsigned int id)
{

    return id - TAR_BLOCK_SIZE;

}

static unsigned int encode(unsigned int address)
{

    return address + TAR_BLOCK_SIZE;

}

static unsigned int protocol_parent(struct vfs_backend *backend, unsigned int id)
{

    struct tar_header header;
    struct tar_header pheader;
    unsigned int address = decode(id);
    unsigned int length;

    if (backend->read(backend, address, TAR_BLOCK_SIZE, &header) < TAR_BLOCK_SIZE)
        return 0;

    length = ascii_length(header.name);

    while (--length && header.name[length - 1] != '/');

    address = 0;

    do
    {

        if (backend->read(backend, address, TAR_BLOCK_SIZE, &pheader) < TAR_BLOCK_SIZE)
            break;

        if (!tar_validate(&pheader))
            break;

        if (pheader.typeflag[0] != TAR_TYPEFLAG_DIRECTORY)
            continue;

        if (memory_match(header.name, pheader.name, length))
            return encode(address);

    } while ((address = tar_next(&pheader, address)));

    return 0;

}

static unsigned int protocol_child(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path)
{

    struct tar_header header;
    unsigned int address = decode(id);
    unsigned int length;

    if (!count)
        return id;

    if (backend->read(backend, address, TAR_BLOCK_SIZE, &header) < TAR_BLOCK_SIZE)
        return 0;

    length = ascii_length(header.name);

    while ((address = tar_next(&header, address)))
    {

        if (backend->read(backend, address, TAR_BLOCK_SIZE, &header) < TAR_BLOCK_SIZE)
            break;

        if (!tar_validate(&header))
            break;

        if ((ascii_length(header.name)) != length + count)
            continue;

        if (memory_match(header.name + length, path, count))
            return encode(address);

    }

    return 0;

}

static unsigned int protocol_create(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *name)
{

    return 0;

}

static unsigned int protocol_destroy(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *name)
{

    return 0;

}

static unsigned int protocol_open(struct vfs_backend *backend, unsigned int id)
{

    return id;

}

static unsigned int protocol_close(struct vfs_backend *backend, unsigned int id)
{

    return id;

}

static unsigned int protocol_read(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    struct tar_header header;
    unsigned int address = decode(id);

    if (backend->read(backend, address, TAR_BLOCK_SIZE, &header) < TAR_BLOCK_SIZE)
        return 0;

    if (header.typeflag[0] == TAR_TYPEFLAG_REGULAR)
    {

        unsigned int s = tar_readvalue(header.size);

        if (offset >= s)
            return 0;

        count = size * count;
        s -= offset;

        return backend->read(backend, address + TAR_BLOCK_SIZE + offset, (count > s) ? s : count, buffer);

    }

    if (header.typeflag[0] == TAR_TYPEFLAG_DIRECTORY)
    {

        struct record *records = buffer;
        unsigned int length = ascii_length(header.name);
        unsigned int i = 0;

        if (offset > 0)
            return 0;

        records[i].length = 3;
        records[i].size = 0;

        memory_read(records[i].name, 120, "../", 3, 1, 0);

        i++;

        while ((address = tar_next(&header, address)))
        {

            if (backend->read(backend, address, TAR_BLOCK_SIZE, &header) < TAR_BLOCK_SIZE)
                break;

            if (!tar_validate(&header))
                break;

            if (protocol_parent(backend, encode(address)) != id)
                continue;

            records[i].length = ascii_length(header.name) - length;
            records[i].size = tar_readvalue(header.size);

            memory_read(records[i].name, 120, header.name + length, records[i].length, 1, 0);

            i++;

        }

        return i;

    }

    return 0;

}

static unsigned int protocol_write(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    struct tar_header header;
    unsigned int address = decode(id);

    if (backend->read(backend, address, TAR_BLOCK_SIZE, &header) < TAR_BLOCK_SIZE)
        return 0;

    if (header.typeflag[0] == TAR_TYPEFLAG_REGULAR)
    {

        unsigned int s = tar_readvalue(header.size);

        if (offset >= s)
            return 0;

        count = size * count;
        s -= offset;

        return backend->write(backend, address + TAR_BLOCK_SIZE + offset, (count > s) ? s : count, buffer);

    }

    return 0;

}

static unsigned long protocol_getphysical(struct vfs_backend *backend, unsigned int id)
{

    /* TEMPORARY FIX */
    return backend->getphysical(backend) + id;

}

void vfs_setuptar()
{

    vfs_initprotocol(&protocol, protocol_match, protocol_root, protocol_parent, protocol_child, protocol_create, protocol_destroy, protocol_open, protocol_close, protocol_read, protocol_write, protocol_getphysical);
    resource_register(&protocol.resource);

}

