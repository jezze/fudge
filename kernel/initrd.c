#include <lib/memory.h>
#include <lib/string.h>
#include <lib/tar.h>
#include <kernel/vfs.h>
#include <kernel/initrd.h>

static struct initrd_filesystem filesystem;

static unsigned int initrd_filesystem_node_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    struct initrd_node *initrdNode = &filesystem.nodes[self->id];

    if (count > initrdNode->size)
        count = initrdNode->size;

    memory_copy(buffer, initrdNode->data, count);

    return count;

}

static struct vfs_node *initrd_filesystem_find_node(struct vfs_filesystem *self, char *name)
{

    unsigned int i;

    for (i = 0; i < filesystem.base.nodeCount; i++)
    {

        if (!filesystem.nodes[i].base.name)
            continue;

        if (string_find(filesystem.nodes[i].base.name, name))
            return &filesystem.nodes[i].base;

    }

    return 0;

}

static struct vfs_node *initrd_filesystem_walk(struct vfs_filesystem *self, unsigned int index)
{

    if (!filesystem.nodes[index].base.name)
        return 0;

    return &filesystem.nodes[index].base;

}

static void initrd_node_init(struct initrd_node *node, unsigned int index, char *name, unsigned int size, struct tar_header *header, void *data)
{

    vfs_node_init(&node->base, name, index, 0, 0, initrd_filesystem_node_read, 0);

    node->size = size;
    node->header = header;
    node->data = data;
    node->base.physical = data;

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

            initrd_node_init(&filesystem.nodes[count], count, header->name, size, header, address + TAR_BLOCK_SIZE);

            count++;

        }

        address += ((size / TAR_BLOCK_SIZE) + 1) * TAR_BLOCK_SIZE;

        if (size % TAR_BLOCK_SIZE)
            address += TAR_BLOCK_SIZE;

    }

    return count;

}

void initrd_init(unsigned int initrdc, void **initrdv)
{

    vfs_filesystem_init(&filesystem.base, initrd_filesystem_find_node, initrd_filesystem_walk);

    filesystem.base.nodeCount = 0;

    unsigned int i;

    for (i = 0; i < initrdc; i++)
        filesystem.base.nodeCount += parse(*(initrdv + i));

    vfs_register_filesystem(&filesystem.base);

}

