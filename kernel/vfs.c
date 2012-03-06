#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/vfs/ramdisk.h>
#include <kernel/vfs/root.h>
#include <kernel/vfs/sys.h>

struct vfs_filesystem *filesystems[VFS_FILESYSTEM_SLOTS];

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

    struct vfs_filesystem *current = 0;
    unsigned int length = 0;

    unsigned int i;

    for (i = 0; i < VFS_FILESYSTEM_SLOTS; i++)
    {

        if (!filesystems[i])
            continue;

        unsigned int l = string_length(filesystems[i]->name);

        if (!memory_compare(filesystems[i]->name, path, l))
        {

            if (l > length)
            {

                current = filesystems[i];
                length = l;

            }

        }

    }

    return (current) ? current : 0;

}

void vfs_filesystem_init(struct vfs_filesystem *filesystem, char *name, void (*open)(struct vfs_filesystem *self, unsigned int id), void (*close)(struct vfs_filesystem *self, unsigned int id), unsigned int (*read)(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*find)(struct vfs_filesystem *self, char *name), void *(*get_physical)(struct vfs_filesystem *self, unsigned int id))
{

    memory_clear(filesystem, sizeof (struct vfs_filesystem));

    filesystem->name = name;
    filesystem->open = open;
    filesystem->close = close;
    filesystem->read = read;
    filesystem->write = write;
    filesystem->find = find;
    filesystem->get_physical = get_physical;

}

void vfs_init()
{

    vfs_root_init();
    vfs_sys_init();
    vfs_ramdisk_init();

}

