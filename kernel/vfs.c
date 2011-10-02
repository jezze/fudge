#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>

static struct vfs_descriptor vfsDescriptors[256];
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

unsigned int vfs_open(char *path)
{

    struct vfs_node *node = vfsFilesystem.lookup(&vfsFilesystem, path);

    if (!node)
        return -1;

    unsigned int i;

    for (i = 0; i < 256; i++)
    {

        if (!vfsDescriptors[i].node)
        {

            vfsDescriptors[i].node = node;
            vfsDescriptors[i].permissions = 0;

            return i;

        }

    }

    return -1;

}

void vfs_close(unsigned int index)
{

    memory_set((void *)&vfsDescriptors[index], 0, sizeof (struct vfs_descriptor));

}

struct vfs_descriptor *vfs_get_descriptor(unsigned int index)
{

    return &vfsDescriptors[index];

}

static unsigned int vfs_node_read(struct vfs_node *node, unsigned int count, void *buffer)
{

    memory_set(buffer, 0, 1);

    unsigned int i;

    for (i = 0; vfsFilesystems[i]; i++)
    {

        if (vfsFilesystems[i] == &vfsFilesystem)
            continue;

        vfsFilesystems[i]->root->operations.read(vfsFilesystems[i]->root, count, buffer + string_length(buffer));

    }

    return string_length(buffer);

}

static struct vfs_node *vfs_filesystem_lookup(struct vfs_filesystem *filesystem, char *path)
{

    if (path[0] != '/')
        return 0;

    if (string_length(path) == 1)
        return vfsFilesystem.root;

    unsigned int i;

    for (i = 0; vfsFilesystems[i]; i++)
    {

        struct vfs_node *node = vfsFilesystems[i]->lookup(vfsFilesystems[i], path + 1);

        if (node)
            return node;

    }

    return 0;

}

void vfs_init()
{

    vfsRoot.operations.read = vfs_node_read;

    string_copy(vfsFilesystem.name, "/");
    vfsFilesystem.root = &vfsRoot;
    vfsFilesystem.lookup = vfs_filesystem_lookup;
    vfs_register_filesystem(&vfsFilesystem);

}

