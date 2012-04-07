#include <lib/memory.h>
#include <lib/string.h>
#include <lib/tar.h>
#include <kernel/modules.h>
#include <kernel/ramdisk.h>
#include <kernel/vfs/ramdisk.h>

static struct ramdisk_node *nodes;
static unsigned int nodesCount;
static struct modules_filesystem filesystem;

static unsigned int read(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    char *in = (char *)buffer;
    struct ramdisk_node *node = &nodes[id - 1];

    if (node->header->typeflag[0] == TAR_FILETYPE_DIR)
    {

        string_write(in, "./\n../\n", 7);

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
            unsigned int slash = memory_index(start, '/', size);

            if (slash != size && slash != size - 1)
                continue;

            string_write(in + length, "%s\n", start);
            length += string_length(in + length);

        }

        return length;

    }
    
    else
    {

        if (offset > node->size)
            return 0;

        unsigned int c = node->size - offset;

        if (c > count)
            c = count;

        memory_copy(in, (char *)node->data + offset, c);

        return c;

    }

    return 0;

}

static unsigned int find(struct modules_filesystem *self, char *name)
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

static void *get_physical(struct modules_filesystem *self, unsigned int id)
{

    return nodes[id - 1].data;

}

void vfs_ramdisk_init(struct ramdisk_node *n, unsigned int c)
{

    nodes = n;
    nodesCount = c;

    modules_filesystem_init(&filesystem, 0x0001, "ramdisk", 0, 0, read, 0, find, get_physical);
    modules_register_filesystem(&filesystem, "/ramdisk/");

}

