#include <fudge/kernel.h>
#include "vfs.h"
#include "kernel.h"

static struct vfs_protocol protocol;

static unsigned int recalculate(unsigned int id)
{

    return (id == protocol.rootid) ? 0 : id;

}

static unsigned int parent(struct vfs_backend *backend, unsigned int id)
{

    unsigned char block[TAR_BLOCK_SIZE];
    unsigned char block2[TAR_BLOCK_SIZE];
    struct tar_header *header = (struct tar_header *)block;
    struct tar_header *header2 = (struct tar_header *)block2;
    unsigned int length;

    id = recalculate(id);

    backend->read(backend, id, TAR_BLOCK_SIZE, block);

    length = string_length(header->name);

    while (--length && header->name[length - 1] != '/');

    id = 0;

    do
    {

        if (!tar_validate(backend->read(backend, id, TAR_BLOCK_SIZE, block2), block2))
            break;

        if (header2->typeflag[0] != TAR_TYPEFLAG_DIRECTORY)
            continue;

        if (memory_match(header->name, header2->name, length))
            return ((id) ? id : protocol.rootid);

    } while ((id = tar_next(header2, id)));

    return 0;

}

static unsigned int match(struct vfs_backend *backend)
{

    unsigned char block[TAR_BLOCK_SIZE];

    return tar_validate(backend->read(backend, 0, TAR_BLOCK_SIZE, block), block);

}

static unsigned int get_physical(struct vfs_backend *backend, unsigned int id)
{

    /* TEMPORARY FIX */

    struct kernel_module *module = (struct kernel_module *)backend;

    return (unsigned int)module->address + recalculate(id) + TAR_BLOCK_SIZE;

}

static unsigned int open(struct vfs_backend *backend, unsigned int id)
{

    return 1;

}

static unsigned int close(struct vfs_backend *backend, unsigned int id)
{

    return 1;

}

static unsigned int read(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned char block[TAR_BLOCK_SIZE];
    struct tar_header *header = (struct tar_header *)block;

    id = recalculate(id);

    backend->read(backend, id, TAR_BLOCK_SIZE, block);

    if (header->typeflag[0] == TAR_TYPEFLAG_REGULAR)
    {

        unsigned int size = string_number(header->size, 8) - offset;

        return backend->read(backend, id + TAR_BLOCK_SIZE + offset, (count > size) ? size : count, buffer);

    }

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

            if (recalculate(parent(backend, id)) != idold)
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

    id = recalculate(id);

    backend->read(backend, id, TAR_BLOCK_SIZE, block);

    if (header->typeflag[0] == TAR_TYPEFLAG_REGULAR)
    {

        unsigned int size = string_number(header->size, 8) - offset;

        return backend->write(backend, id + TAR_BLOCK_SIZE + offset, (count > size) ? size : count, buffer);

    }

    return 0;

}

static unsigned int walk(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path)
{

    unsigned char block[TAR_BLOCK_SIZE];
    struct tar_header *header = (struct tar_header *)block;
    unsigned int n = vfs_findnext(count, path);
    unsigned int length;

    if (!count)
        return ((id) ? id : protocol.rootid);

    id = recalculate(id);

    backend->read(backend, id, TAR_BLOCK_SIZE, block);

    length = string_length(header->name);

    while ((id = tar_next(header, id)))
    {

        if (!tar_validate(backend->read(backend, id, TAR_BLOCK_SIZE, block), block))
            break;

        if (string_length(header->name) != length + n)
            continue;

        if (memory_match(header->name + length, path, n))
            return walk(backend, id, count - n, path + n);

    }

    return 0;

}

struct vfs_protocol *vfs_tar_setup()
{

    vfs_init_protocol(&protocol, 0xFFFFFFFF, match, open, close, read, write, parent, walk, get_physical);

    return &protocol;

}

