#include <fudge/kernel.h>
#include "vfs.h"
#include "vfs_ramdisk.h"

struct vfs_interface ramdisk;

static struct tar_header *next(struct tar_header *header)
{

    unsigned int address;
    unsigned int size;

    if (!header)
        return (struct tar_header *)ramdisk.rootid;

    size = string_read_num(header->size, 8);
    address = (unsigned int)header + ((size / TAR_BLOCK_SIZE) + ((size % TAR_BLOCK_SIZE) ? 2 : 1)) * TAR_BLOCK_SIZE;
    header = (struct tar_header *)address;

    if (tar_validate(header))
        return header;

    return 0;

}

static struct tar_header *parent(struct tar_header *header)
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
            return current;

    }

    return 0;

}

static unsigned int get_physical(struct vfs_interface *self, unsigned int id)
{

    return id + TAR_BLOCK_SIZE;

}

static unsigned int open(struct vfs_interface *self, unsigned int id)
{

    return id;

}

static unsigned int close(struct vfs_interface *self, unsigned int id)
{

    return id;

}

static unsigned int read_directory(struct tar_header *header, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int length = string_length(header->name);
    struct tar_header *current = 0;
    unsigned char *b = buffer;
    unsigned int c = 0;

    c += memory_read(b + c, count - c, "../\n", 4, offset);
    offset -= (offset > 4) ? 4 : offset;

    while ((current = next(current)))
    {

        unsigned int l = string_length(current->name) - length;

        if (current == header)
            continue;

        if (parent(current) != header)
            continue;

        c += memory_read(b + c, count - c, current->name + length, l, offset);
        offset -= (offset > l) ? l : offset;
        c += memory_read(b + c, count - c, "\n", 1, offset);
        offset -= (offset > 1) ? 1 : offset;

    }

    return c;

}

static unsigned int read_file(struct tar_header *header, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int size = string_read_num(header->size, 8);
    unsigned int data = get_physical(&ramdisk, (unsigned int)header);

    return memory_read(buffer, count, (void *)data, size, offset);

}

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct tar_header *header = (struct tar_header *)id;
    unsigned int length = string_length(header->name);

    if (header->name[length - 1] == '/')
        return read_directory(header, offset, count, buffer);

    return read_file(header, offset, count, buffer);

}

static unsigned int write_file(struct tar_header *header, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int size = string_read_num(header->size, 8);
    unsigned int data = get_physical(&ramdisk, (unsigned int)header);

    return memory_write((void *)data, size, buffer, count, offset);

}

static unsigned int write(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct tar_header *header = (struct tar_header *)id;
    unsigned int length = string_length(header->name);

    if (header->name[length - 1] == '/')
        return 0;

    return write_file(header, offset, count, buffer);

}

static unsigned int walk(struct vfs_interface *self, unsigned int id, unsigned int count, const char *path)
{

    struct tar_header *header = (struct tar_header *)id;
    struct tar_header *current = header;
    unsigned int length = string_length(header->name);

    if (!count)
        return id;

    if (memory_match(path, "../", 3))
        return walk(self, (unsigned int)parent(header), count - 3, path + 3);

    while ((current = next(current)))
    {

        unsigned int l = string_length(current->name);

        if (l < length)
            break;

        l -= length;

        if (memory_match(current->name + length, path, l))
            return walk(self, (unsigned int)current, count - l, path + l);

    }

    return 0;

}

struct vfs_interface *vfs_ramdisk_setup(void *address)
{

    vfs_init_interface(&ramdisk, (unsigned int)address, "ramdisk", open, close, read, write, walk, get_physical);

    return &ramdisk;

}

