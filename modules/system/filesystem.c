#include <fudge/memory.h>
#include <fudge/string.h>
#include <kernel/vfs.h>
#include "system.h"
#include "filesystem.h"

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct system_filesystem *filesystem = (struct system_filesystem *)self;
    struct system_node *current;
    unsigned char *b = buffer;
    unsigned int c = 0;

    if (id == 1)
    {

        for (current = &filesystem->groups->base; current; current = current->next)
        {

            c += memory_read(b + c, count - c, current->name, string_length(current->name), offset);

        }

        return c;

    }

    return 0;

}

static unsigned int walk(struct vfs_interface *self, unsigned int id, unsigned int count, const char *path)
{

    if (!count)
        return id;

    return 0;

}

void system_init_filesystem(struct system_filesystem *filesystem)
{

    memory_clear(filesystem, sizeof (struct system_filesystem));

    vfs_init_interface(&filesystem->base, 1, "system", 0, 0, read, 0, walk, 0);

}

