#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <kernel/vfs/root.h>

static struct vfs_mount *mounts;
static struct vfs_filesystem filesystem;

static unsigned int read(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id != 1)
        return 0;

    string_write(buffer, "./\n../\n", 7);

    unsigned int length = 7;
    unsigned int i;


    for (i = 0; i < VFS_MOUNT_SLOTS; i++)
    {

        if (!mounts[i].filesystem)
            continue;

        if (mounts[i].filesystem == self)
            continue;

        string_write(buffer + length, "%s\n", mounts[i].path + 1);
        length += string_length(buffer + length);

    }

    return length;

}

static unsigned int find(struct vfs_filesystem *self, char *name)
{

    return 1;

}

void vfs_root_init(struct vfs_mount *m)
{

    mounts = m;

    vfs_filesystem_init(&filesystem, 0, 0, read, 0, find, 0);
    vfs_mount(&filesystem, "/");

}

