#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <kernel/vfs/root.h>

extern struct vfs_filesystem *filesystems[];

static struct vfs_filesystem filesystem;

static unsigned int read(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id != 1)
        return 0;

    string_write_format(buffer, "./\n../\n", 7);

    unsigned int length = 7;
    unsigned int i;


    for (i = 0; i < VFS_FILESYSTEM_SLOTS; i++)
    {

        if (!filesystems[i])
            continue;

        if (filesystems[i] == self)
            continue;

        string_write_format(buffer + length, "%s\n", filesystems[i]->name + 1);
        length += string_length(buffer + length);

    }

    return length;

}

static unsigned int find(struct vfs_filesystem *self, char *name)
{

    return 1;

}

void vfs_root_init()
{

    vfs_filesystem_init(&filesystem, "/", 0, 0, read, 0, find, 0);
    vfs_register_filesystem(&filesystem);

}

