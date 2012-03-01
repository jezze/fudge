#include <lib/memory.h>
#include <lib/string.h>
#include <lib/tar.h>
#include <kernel/vfs.h>
#include <kernel/initrd.h>

static struct vfs_filesystem filesystem;
static struct initrd_node nodes[INITRD_HEADER_SIZE];
static unsigned int nodesCount;

static unsigned int initrd_filesystem_node_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    struct initrd_node *node = (struct initrd_node *)self;

    if (count > node->size)
        count = node->size;

    memory_copy(buffer, node->data, count);

    return count;

}

static char *initrd_filesystem_get_name(struct vfs_filesystem *self, unsigned int index)
{

    if (index >= nodesCount)
        return 0;

    return nodes[index].base.name;

}

static struct vfs_node *initrd_filesystem_get_node(struct vfs_filesystem *self, unsigned int index)
{

    if (index >= nodesCount)
        return 0;

    return &nodes[index].base;

}

unsigned int initrd_filesystem_find_node(struct vfs_filesystem *self, char *name)
{

    unsigned int i;

    for (i = 0; i < nodesCount; i++)
    {

        if (string_find(nodes[i].base.name, name))
            return i;

    }

    return 0;

}

static unsigned int initrd_filesystem_walk(struct vfs_filesystem *self, unsigned int index)
{

    if (index >= nodesCount - 1)
        return 0;

    return index + 1;

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

    unsigned int count = 0;

    while (*(char *)address)
    {

        struct tar_header *header = address;
        unsigned int size = get_num(header->size);

        if (header->typeflag[0] != TAR_FILETYPE_DIR)
        {

            initrd_node_init(&nodes[count], count, header->name, size, header, address + TAR_BLOCK_SIZE);

            count++;

        }

        address += ((size / TAR_BLOCK_SIZE) + 1) * TAR_BLOCK_SIZE;

        if (size % TAR_BLOCK_SIZE)
            address += TAR_BLOCK_SIZE;

    }

    return count;

}

void initrd_node_init(struct initrd_node *node, unsigned int index, char *name, unsigned int size, struct tar_header *header, void *data)
{

    memory_clear(node, sizeof (struct initrd_node));

    vfs_node_init(&node->base, name, initrd_filesystem_node_read, 0);

    node->size = size;
    node->header = header;
    node->data = data;
    node->base.physical = data;

}

void initrd_init(unsigned int initrdc, void **initrdv)
{

    vfs_filesystem_init(&filesystem, "build/", 0, 0, 0, 0, 0, initrd_filesystem_get_name, initrd_filesystem_get_node, initrd_filesystem_find_node, initrd_filesystem_walk);

    nodesCount = 0;

    unsigned int i;

    for (i = 0; i < initrdc; i++)
        nodesCount += parse(*(initrdv + i));

    vfs_register_filesystem(&filesystem);

}

