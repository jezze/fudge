#include <lib/memory.h>
#include <lib/string.h>
#include <lib/tar.h>
#include <kernel/vfs.h>
#include <kernel/initrd.h>

static struct vfs_view views[8];
static struct initrd_filesystem filesystem;

static unsigned int initrd_filesystem_node_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    struct initrd_node *initrdNode = &filesystem.nodes[self->id];

    if (count > initrdNode->size)
        count = initrdNode->size;

    memory_copy(buffer, initrdNode->data, count);

    return count;

}

static struct vfs_node *initrd_filesystem_view_find_node(struct vfs_view *self, char *name)
{

    unsigned int i;

    for (i = 0; i < filesystem.nodesCount; i++)
    {

        if (!filesystem.nodes[i].base.name[0])
            continue;

        if (!string_find(filesystem.nodes[i].header->name, self->name))
            continue;

        unsigned int count = string_length(filesystem.nodes[i].base.name) + 1;

        if (!memory_compare(name, filesystem.nodes[i].base.name, count))
            return &filesystem.nodes[i].base;

    }

    return 0;

}

static struct vfs_node *initrd_filesystem_view_walk(struct vfs_view *self, unsigned int index)
{

    if (!filesystem.nodes[index].base.name[0])
        return 0;

    if (!string_find(filesystem.nodes[index].header->name, self->name))
        return 0;

    return &filesystem.nodes[index].base;

}

static struct vfs_view *initrd_filesystem_find_view(struct vfs_filesystem *self, char *name)
{

    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        if (!views[i].name[0])
            continue;

        if (!string_compare(views[i].name, name))
            return &views[i];

    }

    return 0;

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
    unsigned int j = 0;

    for (i = 0; ; i++)
    {

        struct tar_header *header = (struct tar_header *)address;

        if (!header->name[0])
            break;

        unsigned int size = get_num(header->size);

        if (header->typeflag[0] == TAR_FILETYPE_DIR)
        {

            address += ((size / TAR_BLOCK_SIZE) + 1) * TAR_BLOCK_SIZE;

            if (size % TAR_BLOCK_SIZE)
                address += TAR_BLOCK_SIZE;

            continue;

        }

        unsigned int start = string_index_reversed(header->name, '/', 0) + 1;

        struct initrd_node *initrdFileNode = &filesystem.nodes[j];
        vfs_node_init(&initrdFileNode->base, j, 0, 0, initrd_filesystem_node_read, 0);
        string_write(initrdFileNode->base.name, header->name + start);
        initrdFileNode->size = size;
        initrdFileNode->header = header;
        initrdFileNode->data = (void *)(address + TAR_BLOCK_SIZE);
        initrdFileNode->base.physical = (void *)(address + TAR_BLOCK_SIZE);

        address += ((size / TAR_BLOCK_SIZE) + 1) * TAR_BLOCK_SIZE;

        if (size % TAR_BLOCK_SIZE)
            address += TAR_BLOCK_SIZE;

        j++;

    }

    return j;

}

void initrd_init(unsigned int initrdc, void **initrdv)
{

    vfs_view_init(&views[0], "bin", initrd_filesystem_view_find_node, initrd_filesystem_view_walk);
    vfs_view_init(&views[1], "boot", initrd_filesystem_view_find_node, initrd_filesystem_view_walk);
    vfs_view_init(&views[2], "grub", initrd_filesystem_view_find_node, initrd_filesystem_view_walk);
    vfs_view_init(&views[3], "home", initrd_filesystem_view_find_node, initrd_filesystem_view_walk);
    vfs_view_init(&views[4], "mod", initrd_filesystem_view_find_node, initrd_filesystem_view_walk);
    vfs_filesystem_init(&filesystem.base, initrd_filesystem_find_view);

    filesystem.nodesCount = 0;

    unsigned int i;

    for (i = 0; i < initrdc; i++)
        filesystem.nodesCount += parse(*(initrdv + i));

    vfs_register_filesystem(&filesystem.base);

}

