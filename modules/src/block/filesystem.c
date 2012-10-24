#include <memory.h>
#include <string.h>
#include <vfs.h>
#include <base/base.h>
#include <block/block.h>

static unsigned int read_root(struct block_filesystem *filesystem, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int o = 0;
    unsigned int i;

    o += memory_write(buffer, count - o, "../\n", 4, o);

    for (i = 0; i < filesystem->interfacesCount; i++)
    {

        string_write_num((char *)buffer + o, i, 10);
        o += string_length((char *)buffer + o);
        o += memory_write(buffer, count - o, "/\n", 2, o);

    }

    return o;

}

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct block_filesystem *filesystem = (struct block_filesystem *)self;

    if (id >= 0x00010000)
    {

        unsigned int type = (id) & 0xFF;
        unsigned int index = (id >> 8) & 0xFF;
        struct block_interface *interface = filesystem->interfaces[index];

        if (type == 0)
            return interface->read_data(interface, offset, count, buffer);

    }

    if (id >= 0x00000100)
        return memory_read(buffer, count, "../\ndata\n", 9, offset);

    if (id == 0x00000001)
        return read_root(filesystem, offset, count, buffer);

    return 0;

}

static unsigned int write(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct block_filesystem *filesystem = (struct block_filesystem *)self;

    if (id >= 0x00010000)
    {

        unsigned int type = (id) & 0xFF;
        unsigned int index = (id >> 8) & 0xFF;
        struct block_interface *interface = filesystem->interfaces[index];

        if (type == 0)
            return interface->write_data(interface, offset, count, buffer);

    }

    return 0;

}

static unsigned int walk(struct vfs_interface *self, unsigned int id, unsigned int count, char *path)
{

    if (!count)
        return id;

    if (id >= 0x00010000)
        return 0;

    if (id >= 0x00000100)
    {

        if (memory_match(path, "../", 3))
            return walk(self, id >> 8, count - 3, path + 3);

        if (memory_match(path, "data", 4))
            return walk(self, (id << 8) + 0, count - 4, path + 4);

    }

    if (id >= 0x00000001)
    {

        if (memory_match(path, "../", 3))
            return walk(self, 1, count - 3, path + 3);

        if (memory_match(path, "0/", 2))
            return walk(self, (id << 8) + 0, count - 2, path + 2);

        if (memory_match(path, "1/", 2))
            return walk(self, (id << 8) + 1, count - 2, path + 2);

        if (memory_match(path, "2/", 2))
            return walk(self, (id << 8) + 2, count - 2, path + 2);

    }

    return 0;

}

void block_init_filesystem(struct block_filesystem *filesystem)
{

    memory_clear(filesystem, sizeof (struct block_filesystem));

    vfs_init_interface(&filesystem->base, 1, "block", 0, 0, read, write, walk, 0);

}

