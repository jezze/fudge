#include <fudge/kernel.h>
#include "kernel.h"
#include "vfs.h"
#include "vfs_tar.h"

static struct vfs_protocol tar;

static struct tar_header *next(struct tar_header *header)
{

    if (!header)
        return (struct tar_header *)tar.rootid;

    header = (struct tar_header *)tar_next(header, (unsigned int)header);

    if (tar_validate(TAR_BLOCK_SIZE, header))
        return header;

    return 0;

}

static unsigned int parent(struct tar_header *header)
{

    unsigned int length = string_length(header->name);
    struct tar_header *current = 0;

    while (--length)
    {

        if (header->name[length - 1] == '/')
            break;

    }

    while ((current = next(current)))
    {

        if (memory_match(current->name, header->name, length))
            return (unsigned int)current;

    }

    return 0;

}

static unsigned int match(struct vfs_backend *backend)
{

    unsigned char block[TAR_BLOCK_SIZE];

    return tar_validate(backend->read(backend, 0, TAR_BLOCK_SIZE, block), block);

}

static unsigned int get_physical(struct vfs_backend *backend, unsigned int id)
{

    return id + TAR_BLOCK_SIZE;

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
    unsigned int length;
    unsigned int size;

    backend->read(backend, id - tar.rootid, TAR_BLOCK_SIZE, block);

    length = string_length(header->name);
    size = string_number(header->size, 8) - offset;

    if (header->name[length - 1] == '/')
    {

        struct tar_header *current = (struct tar_header *)id;
        unsigned char *b = buffer;
        unsigned int c = memory_read(b, count, "../\n", 4, offset);

        header = (struct tar_header *)id;

        offset -= (offset > 4) ? 4 : offset;

        while ((current = next(current)))
        {

            unsigned int l = string_length(current->name) - length;

            if (parent(current) != id)
                continue;

            c += memory_read(b + c, count - c, current->name + length, l, offset);
            offset -= (offset > l) ? l : offset;
            c += memory_read(b + c, count - c, "\n", 1, offset);
            offset -= (offset > 1) ? 1 : offset;

        }

        return c;

    }

    return backend->read(backend, (id - tar.rootid) + TAR_BLOCK_SIZE + offset, (count > size) ? size : count, buffer);

}

static unsigned int write(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned char block[TAR_BLOCK_SIZE];
    struct tar_header *header = (struct tar_header *)block;
    unsigned int length;
    unsigned int size;

    backend->read(backend, id - tar.rootid, TAR_BLOCK_SIZE, block);

    length = string_length(header->name);
    size = string_number(header->size, 8) - offset;

    if (header->name[length - 1] == '/')
        return 0;

    return backend->write(backend, (id - tar.rootid) + TAR_BLOCK_SIZE + offset, (count > size) ? size : count, buffer);

}

static unsigned int walk(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path)
{

    struct tar_header *header = (struct tar_header *)id;
    struct tar_header *current = header;
    unsigned int length = string_length(header->name);

    if (!count)
        return id;

    if (memory_match(path, "../", 3))
        return walk(backend, parent(header), count - 3, path + 3);

    while ((current = next(current)))
    {

        unsigned int l = string_length(current->name);

        if (l < length)
            break;

        l -= length;

        if (memory_match(current->name + length, path, l))
            return walk(backend, (unsigned int)current, count - l, path + l);

    }

    return 0;

}

struct vfs_protocol *vfs_tar_setup(struct kernel_module *module)
{

    vfs_init_protocol(&tar, (unsigned int)module->base, match, open, close, read, write, walk, get_physical);

    return &tar;

}

