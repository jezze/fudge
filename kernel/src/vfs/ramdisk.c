#include <memory.h>
#include <string.h>
#include <tar.h>
#include <modules.h>
#include <ramdisk.h>
#include <vfs/ramdisk.h>

static unsigned int read_file(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct vfs_ramdisk_filesystem *filesystem = (struct vfs_ramdisk_filesystem *)self;
    struct tar_header *header = filesystem->image->headers[id - 1];
    unsigned int size = string_read_num(header->size, 8);
    unsigned int c = (size > offset) ? size - offset : 0;
    unsigned int data = (unsigned int)header + TAR_BLOCK_SIZE;

    if (c > count)
        c = count;

    memory_copy(buffer, (void *)(data + offset), c);

    return c;

}

static unsigned int read(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct vfs_ramdisk_filesystem *filesystem = (struct vfs_ramdisk_filesystem *)self;
    struct tar_header *header = filesystem->image->headers[id - 1];
    unsigned int length = string_length(header->name);

    if (header->name[length - 1] == '/')
    {

        unsigned int i;
        unsigned int c = 0;
        char *out = buffer;

        if (offset > 0)
            return 0;

        for (i = 0; i < filesystem->image->count; i++)
        {

            struct tar_header *current = filesystem->image->headers[i];
            char *start;
            char *slash;
            unsigned int size;

            if (current == header)
                continue;

            if (!memory_find(current->name, header->name, string_length(current->name), string_length(header->name)))
                continue;

            start = current->name + string_length(header->name);
            size = string_length(start);

            if (!size)
                continue;

            slash = memory_find(start, "/", size, 1);

            if (slash && slash < start + size - 1)
                continue;

            memory_copy(out + c, start, size);
            memory_copy(out + c + size, "\n", 1);
            c += size + 1;

        }

        return c;

    }

    return read_file(self, id, offset, count, buffer);

}

static unsigned int write_file(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct vfs_ramdisk_filesystem *filesystem = (struct vfs_ramdisk_filesystem *)self;
    struct tar_header *header = filesystem->image->headers[id - 1];
    unsigned int size = string_read_num(header->size, 8);
    unsigned int c = (size > offset) ? size - offset : 0;
    unsigned int data = (unsigned int)header + TAR_BLOCK_SIZE;

    if (c > count)
        c = count;

    memory_copy((void *)(data + offset), buffer, c);

    return c;

}

static unsigned int write(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct vfs_ramdisk_filesystem *filesystem = (struct vfs_ramdisk_filesystem *)self;
    struct tar_header *header = filesystem->image->headers[id - 1];
    unsigned int length = string_length(header->name);

    if (header->name[length - 1] == '/')
        return 0;

    return write_file(self, id, offset, count, buffer);

}

static unsigned int walk(struct modules_filesystem *self, unsigned int id, unsigned int count, char *path)
{

    struct vfs_ramdisk_filesystem *filesystem = (struct vfs_ramdisk_filesystem *)self;

    unsigned int i;

    if (!count)
        return id;

    for (i = 0; i < filesystem->image->count; i++)
    {

        if (memory_match(filesystem->image->headers[i]->name + 6, path, count))
            return i + 1;

    }

    return 0;

}

static unsigned int get_physical(struct modules_filesystem *self, unsigned int id)
{

    struct vfs_ramdisk_filesystem *filesystem = (struct vfs_ramdisk_filesystem *)self;
    struct tar_header *header = filesystem->image->headers[id - 1];
    unsigned int data = (unsigned int)header + TAR_BLOCK_SIZE;

    return data;

}

void vfs_ramdisk_filesystem_init(struct vfs_ramdisk_filesystem *filesystem, struct ramdisk_image *image)
{

    memory_clear(filesystem, sizeof (struct vfs_ramdisk_filesystem));

    modules_filesystem_init(&filesystem->base, 0x0001, 0, 1, "ramdisk", 0, 0, read, write, walk, get_physical);

    filesystem->image = image;

}

