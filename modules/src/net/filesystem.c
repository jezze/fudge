#include <memory.h>
#include <modules.h>
#include <net/net.h>

static unsigned int read_root(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct net_driver *driver = (struct net_driver *)self->driver;
    unsigned int i;

    if (offset > 0)
        return 0;

    for (i = 0; i < driver->interfacesCount; i++)
    {

        memory_copy(buffer, "eth0/\n", 6);

    }

    return driver->interfacesCount * 6;

}

static unsigned int read_interface_data(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct net_driver *driver = (struct net_driver *)self->driver;
    struct net_interface *interface = driver->interfaces[0];

    return interface->read(interface, offset, count, buffer);

}

static unsigned int read_interface_mac(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct net_driver *driver = (struct net_driver *)self->driver;
    struct net_interface *interface = driver->interfaces[0];

    memory_copy(buffer, interface->mac, 6);

    return 6;

}

static unsigned int read(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id == 1)
        return read_root(self, id, offset, count, buffer);

    if (id == 2)
        return read_interface_data(self, id, offset, count, buffer);

    if (id == 3)
        return read_interface_mac(self, id, offset, count, buffer);

    return 0;

}

static unsigned int write_interface_data(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct net_driver *driver = (struct net_driver *)self->driver;
    struct net_interface *interface = driver->interfaces[0];

    return interface->write(interface, offset, count, buffer);

}

static unsigned int write(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id == 2)
        return write_interface_data(self, id, offset, count, buffer);

    return 0;

}

static unsigned int walk_interface(struct modules_filesystem *self, unsigned int id, unsigned int count, char *path)
{

    if (!count)
        return id;

    if (memory_match(path, "data", 4))
        return 2;

    if (memory_match(path, "mac", 3))
        return 3;

    return 0;

}

static unsigned int walk(struct modules_filesystem *self, unsigned int id, unsigned int count, char *path)
{

    if (!count)
        return id;

    if (memory_match(path, "eth0/", 5))
        return walk_interface(self, id, count - 5, path + 5);

    return 0;

}

void net_filesystem_init(struct modules_filesystem *filesystem, struct modules_driver *driver)
{

    memory_clear(filesystem, sizeof (struct modules_filesystem));

    modules_filesystem_init(filesystem, 0x1001, driver, 1, "net", 0, 0, read, write, 0, walk, 0); 

}

