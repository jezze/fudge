#include <fudge.h>
#include <tar/tar.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"
#include "container.h"
#include "kernel.h"

static struct vfs_protocol protocol;

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

static unsigned int protocol_match(struct vfs_backend *backend)
{

    struct tar_header header;

    if (backend->read(backend, 0, TAR_BLOCK_SIZE, &header) < TAR_BLOCK_SIZE)
        return 0;

    return tar_validate(&header);

}

static unsigned long protocol_getphysical(struct vfs_backend *backend, unsigned int id)
{

    /* TEMPORARY FIX */

    struct kernel_module *module = (struct kernel_module *)backend;

    return (unsigned long)module->address + id;

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

static unsigned int protocol_read(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct tar_header header;
    unsigned int address = decode(id);

    if (backend->read(backend, address, TAR_BLOCK_SIZE, &header) < TAR_BLOCK_SIZE)
        return 0;

    if (header.typeflag[0] == TAR_TYPEFLAG_REGULAR)
    {

        unsigned int size = tar_readvalue(header.size);

        if (offset >= size)
            return 0;

        size -= offset;

        return backend->read(backend, address + TAR_BLOCK_SIZE + offset, (count > size) ? size : count, buffer);

    }

    if (header.typeflag[0] == TAR_TYPEFLAG_DIRECTORY)
    {

        unsigned char *b = buffer;
        unsigned int c = memory_read(b, count, "../\n", 4, offset);
        unsigned int length = ascii_length(header.name);

        offset -= (offset > 4) ? 4 : offset;

        while ((address = tar_next(&header, address)))
        {

            unsigned int l;

            if (backend->read(backend, address, TAR_BLOCK_SIZE, &header) < TAR_BLOCK_SIZE)
                break;

            if (!tar_validate(&header))
                break;

            if (protocol_parent(backend, encode(address)) != id)
                continue;

            l = ascii_length(header.name) - length;
            c += memory_read(b + c, count - c, header.name + length, l, offset);
            offset -= (offset > l) ? l : offset;
            c += memory_read(b + c, count - c, "\n", 1, offset);
            offset -= (offset > 1) ? 1 : offset;

        }

        return c;

    }

    return 0;

}

static unsigned int protocol_write(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct tar_header header;
    unsigned int address = decode(id);

    if (backend->read(backend, address, TAR_BLOCK_SIZE, &header) < TAR_BLOCK_SIZE)
        return 0;

    if (header.typeflag[0] == TAR_TYPEFLAG_REGULAR)
    {

        unsigned int size = tar_readvalue(header.size);

        if (offset >= size)
            return 0;

        size -= offset;

        return backend->write(backend, address + TAR_BLOCK_SIZE + offset, (count > size) ? size : count, buffer);

    }

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

void vfs_setuptar()
{

    vfs_initprotocol(&protocol, protocol_match, protocol_root, protocol_create, protocol_destroy, protocol_open, protocol_close, protocol_read, protocol_write, protocol_parent, protocol_child, protocol_getphysical);
    resource_register(&protocol.resource);

}

