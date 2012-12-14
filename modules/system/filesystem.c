#include <fudge/memory.h>
#include <vfs.h>
#include <base/base.h>
#include "system.h"

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id > 0x00000001)
        return memory_read(buffer, count, "../\n", 4, offset);

    if (id == 1)
        return memory_read(buffer, count, "../\nblock/\nkbd/\nmouse/\nnet/\nnodefs/\nvideo/\n", 43, offset);

    return 0;

}

static unsigned int walk(struct vfs_interface *self, unsigned int id, unsigned int count, const char *path)
{

    if (!count)
        return id;

    if (memory_match(path, "../", 3))
        return walk(self, 1, count - 3, path + 3);

    if (memory_match(path, "block/", 6))
        return walk(self, 2, count - 6, path + 6);

    if (memory_match(path, "kbd/", 4))
        return walk(self, 3, count - 4, path + 4);

    if (memory_match(path, "mouse/", 6))
        return walk(self, 4, count - 6, path + 6);

    if (memory_match(path, "net/", 4))
        return walk(self, 5, count - 4, path + 4);

    if (memory_match(path, "nodefs/", 7))
        return walk(self, 6, count - 7, path + 7);

    if (memory_match(path, "video/", 6))
        return walk(self, 7, count - 6, path + 6);

    return 0;

}

void system_init_filesystem(struct system_filesystem *filesystem)
{

    memory_clear(filesystem, sizeof (struct system_filesystem));

    vfs_init_interface(&filesystem->base, 1, "system", 0, 0, read, 0, walk, 0);

}

