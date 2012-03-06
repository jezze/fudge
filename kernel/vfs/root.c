#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <kernel/vfs/root.h>

extern struct vfs_filesystem *filesystems[];

static struct vfs_filesystem filesystem;

static unsigned int read(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id == 1)
    {

        unsigned int length = 0;
        unsigned int i;

        for (i = 1; i < VFS_FILESYSTEM_SLOTS; i++)
        {

            if (!filesystems[i])
                continue;

            string_write_format(buffer + length, "%s\n", filesystems[i]->name + 1);
            length += string_length(buffer + length);

        }

        return length;

    }

    if (id == 2)
    {

        string_write_format(buffer, "hej\n", 4);

        return 4;

    }

    return 0;

}

static unsigned int find(struct vfs_filesystem *self, char *name)
{

    unsigned int length = string_length(name);

    if (!length)
        return 1;

    return 0;

}

void vfs_root_init()
{

    vfs_filesystem_init(&filesystem, "/root/", 0, 0, read, 0, find, 0);
    vfs_register_filesystem(&filesystem);

}

