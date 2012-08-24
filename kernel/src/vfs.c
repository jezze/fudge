#include <memory.h>
#include <vfs.h>

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

void vfs_unregister_filesystem(struct vfs_filesystem *filesystem)
{

    unsigned int i;

    for (i = 0; i < VFS_FILESYSTEM_SLOTS; i++)
    {

        if (!filesystems[i])
            continue;

        if (filesystems[i] != filesystem)
            continue;

        filesystems[i] = 0;

        break;

    }

}

void vfs_filesystem_init(struct vfs_filesystem *filesystem, unsigned int rootid, char *name, void (*open)(struct vfs_filesystem *self, unsigned int id), void (*close)(struct vfs_filesystem *self, unsigned int id), unsigned int (*read)(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*walk)(struct vfs_filesystem *self, unsigned int id, unsigned int count, char *path), unsigned int (*get_physical)(struct vfs_filesystem *self, unsigned int id))
{

    memory_clear(filesystem, sizeof (struct vfs_filesystem));

    filesystem->rootid = rootid;
    filesystem->open = open;
    filesystem->close = close;
    filesystem->read = read;
    filesystem->write = write;
    filesystem->walk = walk;
    filesystem->get_physical = get_physical;

}

