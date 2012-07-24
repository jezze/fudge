#include <memory.h>
#include <string.h>
#include <tar.h>
#include <modules.h>
#include <ramdisk.h>
#include <vfs/ramdisk.h>

static struct vfs_ramdisk_filesystem filesystem;

static unsigned int read_file(struct ramdisk_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int size = string_read_num(node->header->size, 8);
    unsigned int c = (size > offset) ? size - offset : 0;

    if (c > count)
        c = count;

    memory_copy(buffer, (void *)(node->offset + offset), c);

    return c;

}

static unsigned int read(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct vfs_ramdisk_filesystem *filesystem = (struct vfs_ramdisk_filesystem *)self;
    struct ramdisk_node *node = &filesystem->image->nodes[id - 1];
    unsigned int length = string_length(node->header->name);

    if (node->header->name[length - 1] == '/')
    {

        unsigned int i;
        unsigned int c = 0;
        char *out = buffer;

        if (offset > 0)
            return 0;

        for (i = 0; i < filesystem->image->count; i++)
        {

            struct ramdisk_node *current = &filesystem->image->nodes[i];
            char *start;
            char *slash;
            unsigned int size;

            if (current == node)
                continue;

            if (!memory_find(current->header->name, node->header->name, string_length(current->header->name), string_length(node->header->name)))
                continue;

            start = current->header->name + string_length(node->header->name);
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

    return read_file(node, offset, count, buffer);

}

static unsigned int write_file(struct ramdisk_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int size = string_read_num(node->header->size, 8);
    unsigned int c = (size > offset) ? size - offset : 0;

    if (c > count)
        c = count;

    memory_copy((void *)(node->offset + offset), buffer, c);

    return c;

}

static unsigned int write(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct vfs_ramdisk_filesystem *filesystem = (struct vfs_ramdisk_filesystem *)self;
    struct ramdisk_node *node = &filesystem->image->nodes[id - 1];
    unsigned int length = string_length(node->header->name);

    if (node->header->name[length - 1] == '/')
        return 0;

    return write_file(node, offset, count, buffer);

}

static unsigned int walk(struct modules_filesystem *self, unsigned int id, unsigned int count, void *buffer)
{

    struct vfs_ramdisk_filesystem *filesystem = (struct vfs_ramdisk_filesystem *)self;

    unsigned int i;

    if (!count)
        return 1;

    for (i = 0; i < filesystem->image->count; i++)
    {

        if (memory_match(filesystem->image->nodes[i].header->name + 6, buffer, count))
            return i + 1;

    }

    return 0;

}

static unsigned int get_physical(struct modules_filesystem *self, unsigned int id)
{

    struct vfs_ramdisk_filesystem *filesystem = (struct vfs_ramdisk_filesystem *)self;

    return filesystem->image->nodes[id - 1].offset;

}

void vfs_ramdisk_filesystem_init(struct vfs_ramdisk_filesystem *filesystem, struct ramdisk_image *image)
{

    memory_clear(filesystem, sizeof (struct vfs_ramdisk_filesystem));

    modules_filesystem_init(&filesystem->base, 0x0001, "ramdisk", 0, 0, read, write, 0, walk, get_physical);

    filesystem->image = image;

}

struct modules_filesystem *vfs_ramdisk_setup(struct ramdisk_image *image)
{

    vfs_ramdisk_filesystem_init(&filesystem, image);
    modules_register_filesystem(&filesystem.base);

    return &filesystem.base;

}

