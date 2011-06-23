#include <lib/call.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <lib/tar.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <kernel/initrd.h>

static struct initrd_filesystem initrdFilesystem;

static unsigned int initrd_file_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct tar_header *header = initrdFilesystem.headers[node->id];

    if (offset > node->length)
        return 0;

    if (offset + count > node->length)
        count = node->length - offset;

    unsigned int address = (unsigned int)header + TAR_BLOCK_SIZE;

    memory_copy(buffer, (unsigned char *)(address + offset), count);

    return count;

}

static unsigned int initrd_file_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    struct tar_header *header = initrdFilesystem.headers[node->id];

    if (offset > node->length)
        return 0;

    if (offset + count > node->length)
        count = node->length - offset;

    unsigned int address = (unsigned int)header + TAR_BLOCK_SIZE;

    memory_copy((unsigned char *)(address + offset), buffer, count);

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

static unsigned int initrd_parse(unsigned int address)
{

    unsigned int i;

    for (i = 0; ; i++)
    {

        struct tar_header *header = (struct tar_header *)address;

        if (header->name[0] == '\0')
            break;

        unsigned int size = initrd_get_file_size(header->size);

        initrdFilesystem.headers[i] = header;

        address += ((size / TAR_BLOCK_SIZE) + 1) * TAR_BLOCK_SIZE;

        if (size % TAR_BLOCK_SIZE)
            address += TAR_BLOCK_SIZE;

    }

    return i;

}

static struct vfs_node *initrd_node_walk(struct vfs_node *node, unsigned int index)
{

    return (index < node->length) ? initrdFilesystem.nodes[node->id + index + 1] : 0;

}

static unsigned int initrd_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    node->length++;

    return count;

}

static unsigned int initrd_node_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    memory_set(buffer, 0, 1);
    unsigned int i;

    for (i = 0; i < node->length; i++)
    {

        string_concat(buffer, initrdFilesystem.nodes[node->id + 1 + i]->name);
        string_concat(buffer, "\n");

    }

    return string_length(buffer);

}

static void initrd_create_nodes(unsigned int numEntries)
{

    unsigned int i;

    for (i = 0; i < numEntries; i++)
    {

        struct tar_header *header = initrdFilesystem.headers[i];

        unsigned int size = initrd_get_file_size(header->size);
        unsigned int start = string_index_reversed(header->name, '/', (header->typeflag[0] == TAR_FILETYPE_DIR) ? 1 : 0) + 1;

        struct vfs_node *initrdFileNode = vfs_add_node(header->name + start, size);
        initrdFileNode->id = i;

        initrdFilesystem.nodes[i] = initrdFileNode;

        if (header->typeflag[0] == TAR_FILETYPE_DIR)
        {

            string_replace(initrdFileNode->name, '/', '\0');
            initrdFileNode->walk = initrd_node_walk;
            initrdFileNode->read = initrd_node_read;
            initrdFileNode->write = initrd_node_write;

        }

        else
        {

            initrdFileNode->read = initrd_file_read;
            initrdFileNode->write = initrd_file_write;

        }

        char baseName[256];
        unsigned int offset = string_length("/root");
        memory_copy(baseName, header->name + offset, start - offset);
        baseName[start - offset] = '\0';

        struct vfs_node *rootNode = vfs_find(vfs_get_root(), baseName);
        rootNode->write(rootNode, rootNode->length, 1, initrdFileNode);

    }

}

void initrd_init(unsigned int *address)
{

    string_copy(initrdFilesystem.base.name, "tarfs");
    modules_register_filesystem(&initrdFilesystem.base);

    unsigned int numEntries = initrd_parse(*address + TAR_BLOCK_SIZE);

    initrd_create_nodes(numEntries);

}

