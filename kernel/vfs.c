#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>

static struct vfs_filesystem *filesystems[VFS_FILESYSTEM_SLOTS];

void vfs_register_filesystem(struct vfs_filesystem *filesystem)
{

    unsigned int i;

    for (i = 0; i < VFS_FILESYSTEM_SLOTS; i++)
    {

        if (filesystems[i])
            continue;

        filesystems[i] = filesystem;

        break;

    }

}

struct vfs_filesystem *vfs_get_filesystem(unsigned int index)
{

    if (!filesystems[index])
        return 0;

    return filesystems[index];

}

struct vfs_filesystem *vfs_find_filesystem(char *path)
{

    if (!string_length(path))
        return 0;

    unsigned int i;

    for (i = 0; i < VFS_FILESYSTEM_SLOTS; i++)
    {

        if (!filesystems[i])
            continue;

        struct vfs_filesystem *filesystem = filesystems[i];

        unsigned int id = filesystem->find(filesystem, path);

        if (!id)
            continue;

        return filesystem;

    }

    return 0;

}

void vfs_filesystem_init(struct vfs_filesystem *filesystem, char *name, unsigned int firstId, void (*open)(struct vfs_filesystem *self, unsigned int id), void (*close)(struct vfs_filesystem *self, unsigned int id), unsigned int (*read)(struct vfs_filesystem *self, unsigned int id, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_filesystem *self, unsigned int id, unsigned int count, void *buffer), char *(*get_name)(struct vfs_filesystem *self, unsigned int id), unsigned int (*find)(struct vfs_filesystem *self, char *name), unsigned int (*walk)(struct vfs_filesystem *self, unsigned int id), void *(*get_physical)(struct vfs_filesystem *self, unsigned int id))
{

    memory_clear(filesystem, sizeof (struct vfs_filesystem));

    filesystem->name = name;
    filesystem->firstId = firstId;
    filesystem->open = open;
    filesystem->close = close;
    filesystem->read = read;
    filesystem->write = write;
    filesystem->get_name = get_name;
    filesystem->find = find;
    filesystem->walk = walk;
    filesystem->get_physical = get_physical;

}

