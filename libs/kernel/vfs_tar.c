#include <fudge/kernel.h>
#include "vfs.h"
#include "task.h"
#include "container.h"
#include "kernel.h"

static unsigned int root(struct vfs_backend *backend)
{

    return 0xFFFFFFFF;

}

static unsigned int decode(unsigned int id)
{

    return (id == 0xFFFFFFFF) ? 0 : id;

}

static unsigned int encode(unsigned int address)
{

    return (address) ? address : 0xFFFFFFFF;

}

static unsigned int parent(struct vfs_backend *backend, unsigned int id)
{

    unsigned char block[TAR_BLOCK_SIZE];
    unsigned char pblock[TAR_BLOCK_SIZE];
    struct tar_header *header = (struct tar_header *)block;
    struct tar_header *pheader = (struct tar_header *)pblock;
    unsigned int address = decode(id);
    unsigned int length;

    if (backend->read(backend, address, TAR_BLOCK_SIZE, block) < TAR_BLOCK_SIZE)
        return 0;

    length = string_length(header->name);

    while (--length && header->name[length - 1] != '/');

    address = 0;

    do
    {

        if (backend->read(backend, address, TAR_BLOCK_SIZE, pblock) < TAR_BLOCK_SIZE)
            return 0;

        if (!tar_validate(pblock))
            break;

        if (pheader->typeflag[0] != TAR_TYPEFLAG_DIRECTORY)
            continue;

        if (memory_match(header->name, pheader->name, length))
            return encode(address);

    } while ((address = tar_next(pheader, address)));

    return 0;

}

static unsigned int match(struct vfs_backend *backend)
{

    unsigned char block[TAR_BLOCK_SIZE];

    if (backend->read(backend, 0, TAR_BLOCK_SIZE, block) < TAR_BLOCK_SIZE)
        return 0;

    return tar_validate(block);

}

static unsigned int get_physical(struct vfs_backend *backend, unsigned int id)
{

    /* TEMPORARY FIX */

    struct kernel_module *module = (struct kernel_module *)backend;

    return (unsigned int)module->address + decode(id) + TAR_BLOCK_SIZE;

}

static unsigned int open(struct vfs_backend *backend, unsigned int id, struct vfs_mode *mode)
{

    return id;

}

static unsigned int close(struct vfs_backend *backend, unsigned int id, struct vfs_mode *mode)
{

    return 0;

}

static unsigned int read(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned char block[TAR_BLOCK_SIZE];
    struct tar_header *header = (struct tar_header *)block;
    unsigned int address = decode(id);

    if (backend->read(backend, address, TAR_BLOCK_SIZE, block) < TAR_BLOCK_SIZE)
        return 0;

    if (header->typeflag[0] == TAR_TYPEFLAG_REGULAR)
    {

        unsigned int size = string_number(header->size, 8) - offset;

        return backend->read(backend, address + TAR_BLOCK_SIZE + offset, (count > size) ? size : count, buffer);

    }

    if (header->typeflag[0] == TAR_TYPEFLAG_DIRECTORY)
    {

        unsigned char *b = buffer;
        unsigned int c = memory_read(b, count, "../\n", 4, offset);
        unsigned int length = string_length(header->name);

        offset -= (offset > 4) ? 4 : offset;

        while ((address = tar_next(header, address)))
        {

            unsigned int l;

            if (backend->read(backend, address, TAR_BLOCK_SIZE, block) < TAR_BLOCK_SIZE)
                return 0;

            if (!tar_validate(block))
                break;

            if (parent(backend, encode(address)) != id)
                continue;

            l = string_length(header->name) - length;
            c += memory_read(b + c, count - c, header->name + length, l, offset);
            offset -= (offset > l) ? l : offset;
            c += memory_read(b + c, count - c, "\n", 1, offset);
            offset -= (offset > 1) ? 1 : offset;

        }

        return c;

    }

    return 0;

}

static unsigned int write(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned char block[TAR_BLOCK_SIZE];
    struct tar_header *header = (struct tar_header *)block;
    unsigned int address = decode(id);

    if (backend->read(backend, address, TAR_BLOCK_SIZE, block) < TAR_BLOCK_SIZE)
        return 0;

    if (header->typeflag[0] == TAR_TYPEFLAG_REGULAR)
    {

        unsigned int size = string_number(header->size, 8) - offset;

        return backend->write(backend, address + TAR_BLOCK_SIZE + offset, (count > size) ? size : count, buffer);

    }

    return 0;

}

static unsigned int walk(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path)
{

    unsigned char block[TAR_BLOCK_SIZE];
    struct tar_header *header = (struct tar_header *)block;
    unsigned int address = decode(id);
    unsigned int n = vfs_findnext(count, path);
    unsigned int length;

    if (!count)
        return id;

    if (backend->read(backend, address, TAR_BLOCK_SIZE, block) < TAR_BLOCK_SIZE)
        return 0;

    length = string_length(header->name);

    while ((address = tar_next(header, address)))
    {

        if (backend->read(backend, address, TAR_BLOCK_SIZE, block) < TAR_BLOCK_SIZE)
            return 0;

        if (!tar_validate(block))
            break;

        if (string_length(header->name) != length + n)
            continue;

        if (memory_match(header->name + length, path, n))
            return walk(backend, encode(address), count - n, path + n);

    }

    return 0;

}

void vfs_init_tar(struct vfs_protocol *protocol)
{

    memory_clear(protocol, sizeof (struct vfs_protocol));
    vfs_init_protocol(protocol, match, root, open, close, read, write, parent, walk, get_physical);

}

