#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>

static struct file_node vfsNodes[256];
static struct file_descriptor vfsOpenTable[256];
static unsigned int vfsNodesCount;

static struct file_node *vfsRootEntries[64];

unsigned int vfs_open(char *name, unsigned int permissions)
{

    struct file_node *node = vfs_find(vfs_get_root(), name);

    if (!node)
        return -1;

    unsigned int i;

    for (i = 0; i < 256; i++)
    {

        if (!vfsOpenTable[i].node)
        {

            vfsOpenTable[i].node = node;
            vfsOpenTable[i].permissions = permissions;

            return i;

        }

    }

    return -1;

}

void vfs_close(unsigned int index)
{

    memory_set((void *)&vfsOpenTable[index], 0, sizeof (struct file_descriptor));

}

struct file_node *vfs_find(struct file_node *node, char *path)
{

    unsigned int index = string_index(path, '/', 0);
    unsigned int length = string_length(path);

    if (!index)
        return node;

    struct file_node *current;
    unsigned int i;

    for (i = 0; (current = file_walk(node, i)); i++)
    {

        unsigned int count = string_length(current->name);

        if (index > count)
            count = index;

        if (!memory_compare(path, current->name, count))
            return (index == length) ? current : vfs_find(current, path + index + 1);

    }

    return 0;

}

struct file_node *vfs_get(unsigned int index)
{

    return &vfsOpenTable[index].node;

}

struct file_node *vfs_get_root(struct file_node *node, unsigned int index)
{

    return &vfsNodes[0];

}

static struct file_node *file_node_walk(struct file_node *node, unsigned int index)
{

    return (index < node->length) ? vfsRootEntries[index] : 0;

}

static unsigned int file_node_write(struct file_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    vfsRootEntries[offset] = (struct file_node *)buffer;
    node->length++;

    return count;

}

struct file_node *vfs_add_node(char *name, unsigned int length)
{

    struct file_node *node = &vfsNodes[vfsNodesCount];
    memory_set(node, 0, sizeof (struct file_node));
    string_copy(node->name, name);
    node->cursor = 0;
    node->length = length;

    vfsNodesCount++;

    return node;

}

void vfs_init()
{

    struct file_node *node = vfs_add_node("root", 0);
    node->walk = file_node_walk;
    node->write = file_node_write;

}

