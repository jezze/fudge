#include <lib/memory.h>
#include <lib/string.h>
#include <lib/tar.h>
#include <kernel/vfs.h>
#include <kernel/initrd.h>

static struct vfs_filesystem filesystem;
static struct initrd_node nodes[INITRD_HEADER_SIZE];
static unsigned int nodesCount;

static unsigned int filesystem_read(struct vfs_filesystem *self, unsigned int id, unsigned int count, void *buffer)
{

    struct initrd_node *node = &nodes[id - 1];

    if (node->header->typeflag[0] == TAR_FILETYPE_DIR)
    {

        unsigned int offset = 0;
        unsigned int i;

        for (i = 0; i < nodesCount; i++)
        {

            if (&nodes[i] == node)
                continue;

            if (!string_find(nodes[i].name, node->name))
                continue;

            char *start = nodes[i].name + string_length(node->name);
            unsigned int length = string_length(start);
            unsigned int slash = string_index(start, '/', 0);

            if (slash != length && slash != length - 1)
                continue;

            string_write_format(buffer + offset, "%s\n", start);
            offset += string_length(buffer + offset);

        }

        return offset;

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

static unsigned int filesystem_find(struct vfs_filesystem *self, char *name)
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

static unsigned int filesystem_walk(struct vfs_filesystem *self, unsigned int id)
{

    if (id > nodesCount - 1)
        return 0;

    return id + 1;

}

static char *filesystem_get_name(struct vfs_filesystem *self, unsigned int id)
{

    return nodes[id - 1].name;

}

static void *filesystem_get_physical(struct vfs_filesystem *self, unsigned int id)
{

    return nodes[id - 1].data;

}

static unsigned int get_num(const char *in)
{

    unsigned int size = 0;
    unsigned int j;
    unsigned int count = 1;

    for (j = 11; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);

    return size;

}

static unsigned int parse(void *address)
{

    unsigned int i;

    for (i = 0; *(char *)address; i++)
    {

        struct tar_header *header = address;
        unsigned int size = get_num(header->size);

        initrd_node_init(&nodes[i], header->name + 11, size, header, address + TAR_BLOCK_SIZE);

        address += ((size / TAR_BLOCK_SIZE) + 1) * TAR_BLOCK_SIZE;

        if (size % TAR_BLOCK_SIZE)
            address += TAR_BLOCK_SIZE;

    }

    return i;

}

void initrd_node_init(struct initrd_node *node, char *name, unsigned int size, struct tar_header *header, void *data)
{

    memory_clear(node, sizeof (struct initrd_node));

    node->name = name;
    node->size = size;
    node->header = header;
    node->data = data;

}

void initrd_init(unsigned int initrdc, void **initrdv)
{

    vfs_filesystem_init(&filesystem, "/ramdisk/", 0, 0, filesystem_read, 0, filesystem_find, filesystem_walk, filesystem_get_name, filesystem_get_physical);

    nodesCount = 0;

    unsigned int i;

    for (i = 0; i < initrdc; i++)
        nodesCount += parse(*(initrdv + i));

    vfs_register_filesystem(&filesystem);

}

