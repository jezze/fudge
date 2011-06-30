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

struct vfs_node *vfs_add_node(char *name, unsigned int length)
{

    struct vfs_node *node = &vfsNodes[vfsNodesCount];
    memory_set(node, 0, sizeof (struct vfs_node));
    string_copy(node->name, name);
    node->length = length;

    vfsNodesCount++;

    return node;

}

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

struct vfs_node *vfs_find_root(char *path)
{

    return vfsFilesystem.lookup(&vfsFilesystem, path);

}

struct vfs_node *vfs_get(unsigned int index)
{

    return vfsOpenTable[index].node;

}

struct vfs_node *vfs_get_root()
{

    return &vfsRoot;

}

static unsigned int vfs_node_read(struct vfs_node *node, unsigned int offset, unsigned int count, void *buffer)
{

    memory_set(buffer, 0, 1);

    unsigned int i;

    for (i = 0; vfsFilesystems[i]; i++)
    {

        if (vfsFilesystems[i] == &vfsFilesystem)
            continue;

        string_concat(buffer, vfsFilesystems[i]->name);
        string_concat(buffer, "\n");

    }

    return string_length(buffer);

}

struct vfs_node *vfs_filesystem_lookup(struct vfs_filesystem *filesystem, char *path)
{

    if (path[0] != '/')
        return 0;

    path++;

    unsigned int index = string_index(path, '/', 0);

    if (!index)
        return vfsFilesystem.root;

    unsigned int i;

    for (i = 0; vfsFilesystems[i]; i++)
    {

        if (!memory_compare(path, vfsFilesystems[i]->name, index))
        {

            if (!string_length(path + index + 1))
                return vfsFilesystems[i]->root;

            if (path[index] == '/')
                return vfsFilesystems[i]->lookup(vfsFilesystems[i], path + index + 1);
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
    vfsRoot.operations.read = vfs_node_read;

    string_copy(vfsFilesystem.name, "root");
    vfsFilesystem.root = &vfsRoot;
    vfsFilesystem.lookup = vfs_filesystem_lookup;
    vfs_register_filesystem(&vfsFilesystem);

}

