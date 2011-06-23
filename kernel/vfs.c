#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>

static struct vfs_node vfsNodes[256];
static struct vfs_descriptor vfsOpenTable[256];
static unsigned int vfsNodesCount;

static struct vfs_node *vfsRootEntries[64];

unsigned int vfs_open(char *name)
{

    struct vfs_node *node = vfs_find(vfs_get_root(), name);

    if (!node)
        return -1;

    unsigned int i;

    for (i = 0; i < 256; i++)
    {

        if (!vfsOpenTable[i].node)
        {

            vfsOpenTable[i].node = node;
            vfsOpenTable[i].permissions = 0;

            return i;

        }

    }

    return -1;

}

void vfs_close(unsigned int index)
{

    memory_set((void *)&vfsOpenTable[index], 0, sizeof (struct vfs_descriptor));

}

struct vfs_node *vfs_find(struct vfs_node *node, char *path)
{

    unsigned int index = string_index(path, '/', 0);
    unsigned int length = string_length(path);

    if (!index)
        return node;

    struct vfs_node *current;
    unsigned int i;

    for (i = 0; (current = node->walk(node, i)); i++)
    {

        unsigned int count = string_length(current->name);

        if (index > count)
            count = index;

        if (!memory_compare(path, current->name, count))
            return (index == length) ? current : vfs_find(current, path + index + 1);

    }

    return 0;

}

struct vfs_node *vfs_get(unsigned int index)
{

    return vfsOpenTable[index].node;

}

struct vfs_node *vfs_get_root()
{

    return &vfsNodes[0];

}

static struct vfs_node *file_node_walk(struct vfs_node *node, unsigned int index)
{

    return (index < node->length) ? vfsRootEntries[index] : 0;

}

static unsigned int file_node_write(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    vfsRootEntries[node->length] = (struct vfs_node *)buffer;
    node->length++;

    return count;

}

static unsigned int file_node_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    memory_set(buffer, 0, 1);
    unsigned int i;

    for (i = 0; i < node->length; i++)
    {

        string_concat(buffer, vfsRootEntries[i]->name);
        string_concat(buffer, "\n");

    }

    return string_length(buffer);

}

struct vfs_node *vfs_add_node(char *name, unsigned int length)
{

    struct vfs_node *node = &vfsNodes[vfsNodesCount];
    memory_set(node, 0, sizeof (struct vfs_node));
    string_copy(node->name, name);
    node->length = length;

    vfsNodesCount++;

    return node;

}

void vfs_init()
{

    struct vfs_node *node = vfs_add_node("root", 0);
    node->walk = file_node_walk;
    node->read = file_node_read;
    node->write = file_node_write;

}

