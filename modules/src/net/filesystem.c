#include <memory.h>
#include <string.h>
#include <vfs.h>
#include <base/base.h>
#include <net/net.h>

static unsigned int read_root(struct net_filesystem *filesystem, unsigned int offset, unsigned int count, void *buffer)
{

    char temp[1024];
    char *b = temp;
    unsigned int i;

    memory_copy(b, "../\n", 4);
    b += string_length(b);

    for (i = 0; i < filesystem->interfacesCount; i++)
    {

        string_write_num(b, i, 10);
        b += string_length(b);
        memory_copy(b, "/\n", 2);
        b += string_length(b);

    }

    return vfs_read(temp, (unsigned int)(b - temp), offset, count, buffer);

}

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct net_filesystem *filesystem = (struct net_filesystem *)self;

    if (id >= 0x00010000)
    {

        unsigned int type = (id) & 0xFF;
        unsigned int index = (id >> 8) & 0xFF;
        struct net_interface *interface = filesystem->interfaces[index];

        if (type == 0)
            return interface->read_data(interface, offset, count, buffer);

        if (type == 1)
            return vfs_read(interface->mac, 6, offset, count, buffer);

    }

    if (id >= 0x00000100)
        return vfs_read("../\ndata\nmac\n", 13, offset, count, buffer);

    if (id == 0x00000001)
        return read_root(filesystem, offset, count, buffer);

    return 0;

}

static unsigned int write(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct net_filesystem *filesystem = (struct net_filesystem *)self;

    if (id >= 0x00010000)
    {

        unsigned int type = (id) & 0xFF;
        unsigned int index = (id >> 8) & 0xFF;
        struct net_interface *interface = filesystem->interfaces[index];

        if (type == 0)
            return interface->write_data(interface, offset, count, buffer);

        if (type == 1)
            return vfs_write(interface->mac, 6, offset, count, buffer);

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

        if (memory_match(path, "mac", 3))
            return walk(self, (id << 8) + 1, count - 3, path + 3);

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

void net_filesystem_init(struct net_filesystem *filesystem)
{

    memory_clear(filesystem, sizeof (struct net_filesystem));

    vfs_init_interface(&filesystem->base, 1, "net", 0, 0, read, write, 0, walk, 0);

}

