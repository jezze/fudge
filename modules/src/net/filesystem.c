#include <memory.h>
#include <string.h>
#include <vfs.h>
#include <modules/modules.h>
#include <net/net.h>

static unsigned int read_root(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct net_filesystem *filesystem = (struct net_filesystem *)self;
    unsigned int i;
    unsigned int c = 0;

    for (i = 0; i < filesystem->interfacesCount; i++)
    {

        char num[32];
        unsigned int length;

        string_write_num(num, i, 10);

        length = string_length(num);

        memory_copy((char *)buffer + c, num, length);
        memory_copy((char *)buffer + c + length, "/\n", 2);

        c += length + 2;

    }

    return c;

}

static unsigned int read_interface(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    memory_copy(buffer, "data\nmac\n", 9);

    return 9;

}

static unsigned int read_interface_data(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct net_filesystem *filesystem = (struct net_filesystem *)self;
    struct net_interface *interface = filesystem->interfaces[id];

    return interface->read(interface, offset, count, buffer);

}

static unsigned int read_interface_mac(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct net_filesystem *filesystem = (struct net_filesystem *)self;
    struct net_interface *interface = filesystem->interfaces[id];

    memory_copy(buffer, interface->mac, 6);

    return 6;

}

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id >= 21000)
        return read_interface_mac(self, id - 21000, offset, count, buffer);

    if (id >= 11000)
        return read_interface_data(self, id - 11000, offset, count, buffer);

    if (id >= 1000)
        return read_interface(self, id - 1000, offset, count, buffer);

    if (id == 1)
        return read_root(self, id, offset, count, buffer);

    return 0;

}

static unsigned int write_interface_data(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct net_filesystem *filesystem = (struct net_filesystem *)self;
    struct net_interface *interface = filesystem->interfaces[id];

    return interface->write(interface, offset, count, buffer);

}

static unsigned int write(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id >= 21000)
        return 0;

    if (id >= 11000)
        return write_interface_data(self, id - 11000, offset, count, buffer);

    return 0;

}

static unsigned int walk_interface(struct vfs_interface *self, unsigned int id, unsigned int count, char *path)
{

    if (!count)
        return id;

    if (memory_match(path, "data", 4))
        return id + 10000;

    if (memory_match(path, "mac", 3))
        return id + 20000;

    return 0;

}

static unsigned int walk(struct vfs_interface *self, unsigned int id, unsigned int count, char *path)
{

    if (!count)
        return id;

    if (memory_match(path, "0/", 2))
        return walk_interface(self, 1000, count - 2, path + 2);

    return 0;

}

void net_filesystem_init(struct net_filesystem *filesystem)
{

    memory_clear(filesystem, sizeof (struct net_filesystem));

    vfs_interface_init(&filesystem->base, 1, "net", 0, 0, read, write, walk, 0);

}

