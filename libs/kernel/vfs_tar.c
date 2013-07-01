#include <fudge/kernel.h>
#include "kernel.h"
#include "vfs.h"
#include "vfs_tar.h"

static struct
{

    struct vfs_protocol base;
    unsigned int physical;

} protocol;

static unsigned int parent(struct vfs_backend *backend, unsigned int count, char *path)
{

    unsigned char block[TAR_BLOCK_SIZE];
    struct tar_header *header = (struct tar_header *)block;
    unsigned int offset = 0;

    while (--count && path[count - 1] != '/');

    do
    {

        if (!tar_validate(backend->read(backend, offset, TAR_BLOCK_SIZE, block), block))
            break;

        if (memory_match(header->name, path, count))
            return offset;

    } while ((offset = tar_next(header, offset)));

    return 0;

}

static unsigned int match(struct vfs_backend *backend)
{

    unsigned char block[TAR_BLOCK_SIZE];

    return tar_validate(backend->read(backend, 0, TAR_BLOCK_SIZE, block), block);

}

static unsigned int get_physical(struct vfs_backend *backend, unsigned int id)
{

    return protocol.physical + id + TAR_BLOCK_SIZE;

}

static unsigned int open(struct vfs_backend *backend, unsigned int id)
{

    return id;

}

static unsigned int close(struct vfs_backend *backend, unsigned int id)
{

    return id;

}

static unsigned int read(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned char block[TAR_BLOCK_SIZE];
    struct tar_header *header = (struct tar_header *)block;
    unsigned int size;

    backend->read(backend, id, TAR_BLOCK_SIZE, block);

    size = string_number(header->size, 8) - offset;

    if (header->typeflag[0] == TAR_TYPEFLAG_REGULAR)
        return backend->read(backend, id + TAR_BLOCK_SIZE + offset, (count > size) ? size : count, buffer);

    if (header->typeflag[0] == TAR_TYPEFLAG_DIRECTORY)
    {

        unsigned char *b = buffer;
        unsigned int c = memory_read(b, count, "../\n", 4, offset);
        unsigned int length = string_length(header->name);
        unsigned int idold = id;

        offset -= (offset > 4) ? 4 : offset;

        while ((id = tar_next(header, id)))
        {

            unsigned int l;

            if (!tar_validate(backend->read(backend, id, TAR_BLOCK_SIZE, block), block))
                break;

            if (parent(backend, string_length(header->name), header->name) != idold)
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
    unsigned int size;

    backend->read(backend, id, TAR_BLOCK_SIZE, block);

    size = string_number(header->size, 8) - offset;

    if (header->typeflag[0] == TAR_TYPEFLAG_REGULAR)
        return backend->write(backend, id + TAR_BLOCK_SIZE + offset, (count > size) ? size : count, buffer);

    return 0;

}

static unsigned int walk(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path)
{

    unsigned char block[TAR_BLOCK_SIZE];
    struct tar_header *header = (struct tar_header *)block;
    unsigned int length;

    backend->read(backend, id, TAR_BLOCK_SIZE, block);

    length = string_length(header->name);

    if (!count)
        return id;

    if (memory_match(path, "../", 3))
        return walk(backend, parent(backend, length, header->name), count - 3, path + 3);

    while ((id = tar_next(header, id)))
    {

        unsigned int l;

        if (!tar_validate(backend->read(backend, id, TAR_BLOCK_SIZE, block), block))
            break;

        l = string_length(header->name);

        if (l < length)
            break;

        l -= length;

        if (memory_match(header->name + length, path, l))
            return walk(backend, id, count - l, path + l);

    }

    return 0;

}

struct vfs_protocol *vfs_tar_setup(unsigned int physical)
{

    vfs_init_protocol(&protocol.base, 0, match, open, close, read, write, walk, get_physical);

    protocol.physical = physical;

    return &protocol.base;

}

