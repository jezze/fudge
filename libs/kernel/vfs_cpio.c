#include <kernel.h>
#include <cpio/cpio.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"
#include "container.h"
#include "kernel.h"

static unsigned int root(struct vfs_backend *backend)
{

    struct cpio_header header;
    unsigned int address = 0;
    unsigned int current = 0;
    unsigned short namesize = 0xFFFF;

    do
    {

        if (backend->read(backend, address, sizeof (struct cpio_header), &header) < sizeof (struct cpio_header))
            break;

        if (!cpio_validate(&header))
            break;

        if ((header.mode & 0xF000) != 0x4000)
            continue;

        if (header.namesize < namesize)
        {

            namesize = header.namesize;
            current = address;

        }

    } while ((address = cpio_next(&header, address)));

    return current;

}

static unsigned int decode(struct vfs_backend *backend, unsigned int id)
{

    return (id == 0xFFFFFFFF) ? root(backend) : id;

}

static unsigned int encode(unsigned int address)
{

    return (address) ? address : 0xFFFFFFFF;

}

static unsigned int parent(struct vfs_backend *backend, unsigned int id)
{

    struct cpio_header header;
    unsigned char name[1024];
    unsigned int address = decode(backend, id);
    unsigned int length;

    if (backend->read(backend, address, sizeof (struct cpio_header), &header) < sizeof (struct cpio_header))
        return 0;

    if (backend->read(backend, address + sizeof (struct cpio_header), header.namesize, name) < header.namesize)
        return 0;

    length = header.namesize;

    while (--length && name[length - 1] != '/');

    address = 0;

    do
    {

        unsigned char pname[1024];

        if (backend->read(backend, address, sizeof (struct cpio_header), &header) < sizeof (struct cpio_header))
            break;

        if (!cpio_validate(&header))
            break;

        if ((header.mode & 0xF000) != 0x4000)
            continue;

        if (header.namesize != length - 1)
            continue;

        if (backend->read(backend, address + sizeof (struct cpio_header), header.namesize, pname) < header.namesize)
            break;

        if (memory_match(name, pname, length - 1))
            return encode(address);

    } while ((address = cpio_next(&header, address)));

    return 0;

}

static unsigned int match(struct vfs_backend *backend)
{

    struct cpio_header header;

    if (backend->read(backend, 0, sizeof (struct cpio_header), &header) < sizeof (struct cpio_header))
        return 0;

    return cpio_validate(&header);

}

static unsigned long get_physical(struct vfs_backend *backend, unsigned int id)
{

    /* TEMPORARY FIX */

    struct kernel_module *module = (struct kernel_module *)backend;
    struct cpio_header header;
    unsigned int address = decode(backend, id);

    if (backend->read(backend, address, sizeof (struct cpio_header), &header) < sizeof (struct cpio_header))
        return 0;

    return (unsigned long)module->address + address + sizeof (struct cpio_header) + header.namesize + (header.namesize & 1);

}

static unsigned int open(struct vfs_backend *backend, unsigned int id)
{

    return id;

}

static unsigned int close(struct vfs_backend *backend, unsigned int id)
{

    return 0;

}

static unsigned int read(struct vfs_backend *backend, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct cpio_header header;
    unsigned int address = decode(backend, id);

    if (backend->read(backend, address, sizeof (struct cpio_header), &header) < sizeof (struct cpio_header))
        return 0;

    if ((header.mode & 0xF000) == 0x8000)
    {

        unsigned int size = ((header.filesize[0] << 16) | header.filesize[1]) - offset;

        return backend->read(backend, address + sizeof (struct cpio_header) + header.namesize + (header.namesize & 1) + offset, (count > size) ? size : count, buffer);

    }

    if ((header.mode & 0xF000) == 0x4000)
    {

        unsigned char *b = buffer;
        unsigned int c = memory_read(b, count, "../\n", 4, offset);
        unsigned int length = header.namesize;

        offset -= (offset > 4) ? 4 : offset;

        while ((address = cpio_next(&header, address)))
        {

            unsigned char name[1024];
            unsigned int l;

            if (backend->read(backend, address, sizeof (struct cpio_header), &header) < sizeof (struct cpio_header))
                break;

            if (!cpio_validate(&header))
                break;

            if (backend->read(backend, address + sizeof (struct cpio_header), header.namesize, name) < header.namesize)
                break;

            if (parent(backend, encode(address)) != id)
                continue;

            l = header.namesize - length;
            c += memory_read(b + c, count - c, name + length, l, offset);
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

    struct cpio_header header;
    unsigned int address = decode(backend, id);

    if (backend->read(backend, address, sizeof (struct cpio_header), &header) < sizeof (struct cpio_header))
        return 0;

    if ((header.mode & 0xF000) == 0x8000)
    {

        unsigned int size = ((header.filesize[0] << 16) | header.filesize[1]) - offset;

        return backend->write(backend, address + sizeof (struct cpio_header) + header.namesize + (header.namesize & 1) + offset, (count > size) ? size : count, buffer);

    }

    return 0;

}

static unsigned int walk(struct vfs_backend *backend, unsigned int id, unsigned int count, const char *path)
{

    struct cpio_header header;
    unsigned int address = decode(backend, id);
    unsigned int n = vfs_findnext(count, path);
    unsigned int length;

    if (!count)
        return id;

    if (backend->read(backend, address, sizeof (struct cpio_header), &header) < sizeof (struct cpio_header))
        return 0;

    length = header.namesize - 1;
    address = 0;

    do
    {

        unsigned char name[1024];

        if (backend->read(backend, address, sizeof (struct cpio_header), &header) < sizeof (struct cpio_header))
            break;

        if (!cpio_validate(&header))
            break;

        if ((header.mode & 0xF000) == 0x0000)
            continue;

        if (backend->read(backend, address + sizeof (struct cpio_header), header.namesize, name) < header.namesize)
            break;

        if ((header.mode & 0xF000) == 0x8000)
        {

            if ((header.namesize - 1) - length != n + 1)
                continue;

            if (memory_match(name + length + 1, path, n))
                return walk(backend, encode(address), count - n, path + n);

        }

        if ((header.mode & 0xF000) == 0x4000)
        {

            if ((header.namesize - 1) - length != n)
                continue;

            if (memory_match(name + length + 1, path, n - 1))
                return walk(backend, encode(address), count - n, path + n);

        }

    } while ((address = cpio_next(&header, address)));

    return 0;

}

void vfs_setup_cpio(struct vfs_protocol *protocol)
{

    vfs_init_protocol(protocol, match, root, open, close, read, write, parent, walk, get_physical);
    resource_register_item(&protocol->resource);

}

