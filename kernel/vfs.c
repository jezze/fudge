#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>

static struct vfs_node vfsNodes[256];
static struct vfs_descriptor vfsOpenTable[256];
static unsigned int vfsNodesCount;

static struct vfs_node *vfsRootEntries[64];

static struct vfs_filesystem *vfsFilesystems[8];
static struct vfs_filesystem vfsFilesystem;
static struct vfs_node vfsRoot;

void vfs_register_filesystem(struct vfs_filesystem *filesystem)
{

    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        if (!vfsFilesystems[i])
        {

            vfsFilesystems[i] = filesystem;

            return;

        }

    }

}

unsigned int vfs_open(char *name)
{

    struct vfs_node *node = vfs_find_root(name);

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

struct vfs_node *vfs_find_root(char *path)
{

    struct vfs_node *t = vfsFilesystem.lookup(&vfsFilesystem, path);

/*
    return t;

    if (t)
    {

        struct vfs_node *out = vfs_get(1);

        if (out)
        {

        out->write(out, 0, 7, "FOUND: ");
        out->write(out, 0, string_length(t->name), t->name);
        out->write(out, 0, 1, "\n");

        }

    }
*/

    return vfs_find(vfs_get_root(), path + 1);

}

struct vfs_node *vfs_get(unsigned int index)
{

    return vfsOpenTable[index].node;

}

struct vfs_node *vfs_get_root()
{

    return &vfsRoot;

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

struct vfs_node *vfs_filesystem_lookup(struct vfs_filesystem *filesystem, char *path)
{

    if (string_length(path) == 1 && path[0] == '/')
        return vfsFilesystem.root;

    unsigned int i;

    for (i = 0; vfsFilesystems[i]; i++)
    {

        if (!memory_compare(path + 1, vfsFilesystems[i]->root->name, string_length(vfsFilesystems[i]->root->name)))
        {

            if (path[string_length(vfsFilesystems[i]->root->name) + 1] == '/')
                return vfsFilesystems[i]->lookup(vfsFilesystems[i], path + string_length(vfsFilesystems[i]->root->name) + 2);
            else
                return vfsFilesystems[i]->root;

        }

    }

    return 0;

}

void vfs_init()
{

    string_copy(vfsRoot.name, "root");
    vfsRoot.length = 0;
    vfsRoot.walk = file_node_walk;
    vfsRoot.read = file_node_read;
    vfsRoot.write = file_node_write;

    string_copy(vfsFilesystem.name, "rootfs");
    vfsFilesystem.root = &vfsRoot;
    vfsFilesystem.lookup = vfs_filesystem_lookup;
    vfs_register_filesystem(&vfsFilesystem);

}

