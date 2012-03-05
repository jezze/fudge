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

struct vfs_filesystem *vfs_find_filesystem(char *path)
{

    unsigned int i;

    for (i = 0; i < VFS_FILESYSTEM_SLOTS; i++)
    {

        if (!filesystems[i])
            continue;

        if (memory_compare(filesystems[i]->name, path, string_length(filesystems[i]->name)))
            continue;

        return filesystems[i];

    }

    return 0;

}

void vfs_filesystem_init(struct vfs_filesystem *filesystem, char *name, void (*open)(struct vfs_filesystem *self, unsigned int id), void (*close)(struct vfs_filesystem *self, unsigned int id), unsigned int (*read)(struct vfs_filesystem *self, unsigned int id, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_filesystem *self, unsigned int id, unsigned int count, void *buffer), unsigned int (*find)(struct vfs_filesystem *self, char *name), unsigned int (*walk)(struct vfs_filesystem *self, unsigned int id), char *(*get_name)(struct vfs_filesystem *self, unsigned int id), void *(*get_physical)(struct vfs_filesystem *self, unsigned int id))
{

    memory_clear(filesystem, sizeof (struct vfs_filesystem));

    filesystem->name = name;
    filesystem->open = open;
    filesystem->close = close;
    filesystem->read = read;
    filesystem->write = write;
    filesystem->find = find;
    filesystem->walk = walk;
    filesystem->get_name = get_name;
    filesystem->get_physical = get_physical;

}

