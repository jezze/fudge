#include <fudge/kernel.h>
#include "vfs.h"
#include "vfs_ramdisk.h"

#define RAMDISK_HEADER_SLOTS            128

struct ramdisk_filesystem
{

    struct vfs_interface interface;
    struct {struct tar_header *headers[RAMDISK_HEADER_SLOTS]; unsigned int count;} image;

} iramdisk;

static unsigned int open(struct vfs_interface *self, unsigned int id)
{

    return id;

}

static unsigned int close(struct vfs_interface *self, unsigned int id)
{

    return id;

}

static unsigned int parent(struct ramdisk_filesystem *filesystem, unsigned int id)
{

    struct tar_header *header = filesystem->image.headers[id - 1];
    unsigned int length = string_length(header->name);
    unsigned int i;

    while (--length)
    {

        if (header->name[length - 1] == '/')
            break;

    }

    for (i = 0; i < id - 1; i++)
    {

        if (memory_match(filesystem->image.headers[i]->name, header->name, length))
            return i + 1;

    }

    return 0;

}

static unsigned int read_directory(struct ramdisk_filesystem *filesystem, struct tar_header *header, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int length = string_length(header->name);
    unsigned char *b = buffer;
    unsigned int c = 0;
    unsigned int i;

    c += memory_read(b + c, count - c, "../\n", 4, offset);
    offset -= (offset > 4) ? 4 : offset;

    for (i = 0; i < filesystem->image.count; i++)
    {

        unsigned int p = parent(filesystem, i + 1) - 1;
        unsigned int l = string_length(filesystem->image.headers[i]->name);

        if (filesystem->image.headers[i] == header)
            continue;

        if (filesystem->image.headers[p] != header)
            continue;

        c += memory_read(b + c, count - c, filesystem->image.headers[i]->name + length, l - length, offset);
        offset -= (offset > l - length) ? l - length : offset;
        c += memory_read(b + c, count - c, "\n", 1, offset);
        offset -= (offset > 1) ? 1 : offset;

    }

    return c;

}

static unsigned int read_file(struct tar_header *header, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int size = string_read_num(header->size, 8);
    unsigned int data = (unsigned int)header + TAR_BLOCK_SIZE;

    return memory_read(buffer, count, (void *)data, size, offset);

}

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct ramdisk_filesystem *filesystem = (struct ramdisk_filesystem *)self;
    struct tar_header *header = filesystem->image.headers[id - 1];
    unsigned int length = string_length(header->name);

    if (header->name[length - 1] == '/')
        return read_directory(filesystem, header, offset, count, buffer);

    return read_file(header, offset, count, buffer);

}

static unsigned int write_file(struct tar_header *header, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int size = string_read_num(header->size, 8);
    unsigned int data = (unsigned int)header + TAR_BLOCK_SIZE;

    return memory_write((void *)data, size, buffer, count, offset);

}

static unsigned int write(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct ramdisk_filesystem *filesystem = (struct ramdisk_filesystem *)self;
    struct tar_header *header = filesystem->image.headers[id - 1];
    unsigned int length = string_length(header->name);

    if (header->name[length - 1] == '/')
        return 0;

    return write_file(header, offset, count, buffer);

}

static unsigned int walk(struct vfs_interface *self, unsigned int id, unsigned int count, const char *path)
{

    struct ramdisk_filesystem *filesystem = (struct ramdisk_filesystem *)self;
    struct tar_header *header = filesystem->image.headers[id - 1];
    unsigned int length = string_length(header->name);
    unsigned int i;

    if (!count)
        return id;

    if (memory_match(path, "../", 3))
        return walk(self, parent(filesystem, id), count - 3, path + 3);

    for (i = id; i < filesystem->image.count; i++)
    {

        unsigned int l = string_length(filesystem->image.headers[i]->name);

        if (l < length)
            break;

        l -= length;

        if (memory_match(filesystem->image.headers[i]->name + length, path, l))
            return walk(self, i + 1, count - l, path + l);

    }

    return 0;

}

static unsigned int get_physical(struct vfs_interface *self, unsigned int id)
{

    struct ramdisk_filesystem *filesystem = (struct ramdisk_filesystem *)self;
    struct tar_header *header = filesystem->image.headers[id - 1];
    unsigned int data = (unsigned int)header + TAR_BLOCK_SIZE;

    return data;

}

static unsigned int parse(struct ramdisk_filesystem *filesystem, void *address)
{

    char *current;

    for (current = address; *current; current += TAR_BLOCK_SIZE)
    {

        struct tar_header *header = (struct tar_header *)current;
        unsigned int size = string_read_num(header->size, 8);

        if (!tar_validate(header))
            return 0;

        filesystem->image.headers[filesystem->image.count] = header;
        filesystem->image.count++;
        current += ((size / TAR_BLOCK_SIZE) + ((size % TAR_BLOCK_SIZE) ? 1 : 0)) * TAR_BLOCK_SIZE;

    }

    return filesystem->image.count;

}

struct vfs_interface *vfs_ramdisk_setup(void *address)
{

    memory_clear(&iramdisk, sizeof (struct ramdisk_filesystem));
    vfs_init_interface(&iramdisk.interface, 1, "ramdisk", open, close, read, write, walk, get_physical);
    parse(&iramdisk, address);

    return &iramdisk.interface;

}

