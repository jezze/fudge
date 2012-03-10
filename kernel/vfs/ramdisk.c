#include <lib/memory.h>
#include <lib/string.h>
#include <lib/tar.h>
#include <kernel/vfs.h>
#include <kernel/initrd.h>
#include <kernel/vfs/ramdisk.h>

extern struct initrd_node nodes[];
extern unsigned int nodesCount;

static struct vfs_filesystem filesystem;

static unsigned int read(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct initrd_node *node = &nodes[id - 1];

    if (node->header->typeflag[0] == TAR_FILETYPE_DIR)
    {

        string_write_format(buffer, "./\n../\n", 7);

        unsigned int length = 7;
        unsigned int i;

        for (i = 0; i < nodesCount; i++)
        {

            if (&nodes[i] == node)
                continue;

            if (!string_find(nodes[i].name, node->name))
                continue;

            char *start = nodes[i].name + string_length(node->name);
            unsigned int size = string_length(start);
            unsigned int slash = memory_index(start, '/', size, 0);

            if (slash != size && slash != size - 1)
                continue;

            string_write_format(buffer + length, "%s\n", start);
            length += string_length(buffer + length);

        }

        return length;

    }
    
    else
    {

        if (count > node->size)
            count = node->size;

        memory_copy(buffer, node->data, count);

        return count;

    }

    return 0;

}

static unsigned int find(struct vfs_filesystem *self, char *name)
{

    unsigned int length = string_length(name);

    if (!length)
        return 1;

    unsigned int i;

    for (i = 0; i < nodesCount; i++)
    {

        unsigned int l2 = string_length(nodes[i].name);

        if (length != l2)
            continue;

        if (!memory_compare(nodes[i].name, name, length))
            return i + 1;

    }

    return 0;

}

static void *get_physical(struct vfs_filesystem *self, unsigned int id)
{

    return nodes[id - 1].data;

}

void vfs_ramdisk_init()
{

    vfs_filesystem_init(&filesystem, 0, 0, read, 0, find, get_physical);
    vfs_mount(&filesystem, "/ramdisk/");

}

