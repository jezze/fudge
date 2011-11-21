#include <lib/memory.h>
#include <lib/string.h>
#include <lib/tar.h>
#include <kernel/vfs.h>
#include <kernel/initrd.h>

static struct vfs_view initrdViews[8];
static struct initrd_filesystem initrdFilesystem;

static unsigned int initrd_node_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    struct initrd_node *initrdNode = &initrdFilesystem.nodes[self->id];

    if (count > initrdNode->size)
        count = initrdNode->size;

    memory_copy(buffer, initrdNode->data, count);

    return count;

}

static unsigned int initrd_get_file_size(const char *in)
{

    unsigned int size = 0;
    unsigned int j;
    unsigned int count = 1;

    for (j = 11; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);

    return size;

}

static unsigned int initrd_parse(void *address)
{

    unsigned int i;

    for (i = 0; ; i++)
    {

        struct tar_header *header = (struct tar_header *)address;

        if (!header->name[0])
            break;

        unsigned int size = initrd_get_file_size(header->size);
        unsigned int start = string_index_reversed(header->name, '/', (header->typeflag[0] == TAR_FILETYPE_DIR) ? 1 : 0) + 1;

        struct initrd_node *initrdFileNode = &initrdFilesystem.nodes[i];
        vfs_node_init(&initrdFileNode->base, i, 0, 0, initrd_node_read, 0);
        string_write(initrdFileNode->base.name, header->name + start);
        initrdFileNode->size = size;
        initrdFileNode->header = header;
        initrdFileNode->data = (void *)(address + TAR_BLOCK_SIZE);
        initrdFileNode->base.physical = (void *)(address + TAR_BLOCK_SIZE);

        address += ((size / TAR_BLOCK_SIZE) + 1) * TAR_BLOCK_SIZE;

        if (size % TAR_BLOCK_SIZE)
            address += TAR_BLOCK_SIZE;

    }

    return i;

}

static struct vfs_node *initrd_filesystem_view_find_node(struct vfs_view *self, char *name)
{

    unsigned int i;

    for (i = 0; i < initrdFilesystem.nodesCount; i++)
    {

        if (!initrdFilesystem.nodes[i].base.name[0])
            continue;

        if (!string_find(initrdFilesystem.nodes[i].header->name, self->name))
            continue;

        unsigned int count = string_length(initrdFilesystem.nodes[i].base.name) + 1;

        if (!memory_compare(name, initrdFilesystem.nodes[i].base.name, count))
            return &initrdFilesystem.nodes[i].base;

    }

    return 0;

}

static struct vfs_node *initrd_filesystem_view_walk(struct vfs_view *self, unsigned int index)
{

    if (!initrdFilesystem.nodes[index].base.name[0])
        return 0;

    if (!string_find(initrdFilesystem.nodes[index].header->name, self->name))
        return 0;

    return &initrdFilesystem.nodes[index].base;

}

static struct vfs_view *initrd_filesystem_find_view(struct vfs_filesystem *self, char *name)
{

    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        if (!initrdViews[i].name[0])
            continue;

        if (!string_compare(initrdViews[i].name, name))
            return &initrdViews[i];

    }

    return 0;

}

void initrd_init(unsigned int initrdc, void **initrdv)
{

    vfs_view_init(&initrdViews[0], "bin", initrd_filesystem_view_find_node, initrd_filesystem_view_walk);
    vfs_view_init(&initrdViews[1], "boot", initrd_filesystem_view_find_node, initrd_filesystem_view_walk);
    vfs_view_init(&initrdViews[2], "grub", initrd_filesystem_view_find_node, initrd_filesystem_view_walk);
    vfs_view_init(&initrdViews[3], "home", initrd_filesystem_view_find_node, initrd_filesystem_view_walk);
    vfs_view_init(&initrdViews[4], "mod", initrd_filesystem_view_find_node, initrd_filesystem_view_walk);
    vfs_filesystem_init(&initrdFilesystem.base, initrd_filesystem_find_view);

    initrdFilesystem.nodesCount = 0;

    unsigned int i;

    for (i = 0; i < initrdc; i++)
        initrdFilesystem.nodesCount += initrd_parse(*(initrdv + i));

    vfs_register_filesystem(&initrdFilesystem.base);

}

