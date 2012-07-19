#include <memory.h>
#include <string.h>
#include <tar.h>
#include <modules.h>
#include <ramdisk.h>
#include <vfs/ramdisk.h>

static struct vfs_ramdisk_filesystem filesystem;

static unsigned int read(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct vfs_ramdisk_filesystem *filesystem = (struct vfs_ramdisk_filesystem *)self;
    struct ramdisk_node *node = &filesystem->image->nodes[id - 1];

    if (node->header->typeflag[0] == TAR_FILETYPE_DIR)
    {

        unsigned int i;
        unsigned int length = 0;
        char *out = buffer;

        for (i = 0; i < filesystem->image->count; i++)
        {

            char *start;
            char *slash;
            unsigned int size;

            if (&filesystem->image->nodes[i] == node)
                continue;

            if (!memory_find(filesystem->image->nodes[i].name, node->name, string_length(filesystem->image->nodes[i].name), string_length(node->name)))
                continue;

            start = filesystem->image->nodes[i].name + string_length(node->name);
            size = string_length(start);
            slash = memory_find(start, "/", size, 1);

            if (slash && slash < start + size)
                continue;

            memory_copy(out + length, start, size);
            memory_copy(out + length + size, "\n", 1);
            length += size + 1;

        }

        return length;

    }
    
    else
    {

        unsigned int c;

        if (offset > node->size)
            return 0;

        c = node->size - offset;

        if (c > count)
            c = count;

        memory_copy(buffer, (void *)(node->offset + offset), c);

        return c;

    }

    return 0;

}

static unsigned int write(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct vfs_ramdisk_filesystem *filesystem = (struct vfs_ramdisk_filesystem *)self;
    struct ramdisk_node *node = &filesystem->image->nodes[id - 1];
    unsigned int c;

    if (node->header->typeflag[0] == TAR_FILETYPE_DIR)
        return 0;

    if (offset > node->size)
        return 0;

    c = node->size - offset;

    if (c > count)
        c = count;

    memory_copy((void *)(node->offset + offset), buffer, c);

    return c;

}

static unsigned int walk(struct modules_filesystem *self, unsigned int id, unsigned int count, void *buffer)
{

    struct vfs_ramdisk_filesystem *filesystem = (struct vfs_ramdisk_filesystem *)self;

    unsigned int i;

    if (!count)
        return 1;

    for (i = 0; i < filesystem->image->count; i++)
    {

        if (memory_compare(filesystem->image->nodes[i].name, buffer, count))
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

