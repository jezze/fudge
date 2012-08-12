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

        memory_copy(buffer, "eth0\n", 5);

    }

    return driver->interfacesCount * 5;

}

static unsigned int read_interface(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct net_driver *driver = (struct net_driver *)self->driver;
    struct net_interface *interface = driver->interfaces[id - 2];

    return interface->read(interface, offset, count, buffer);

}

static unsigned int read(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id == 1)
        return read_root(self, id, offset, count, buffer);

    if (id > 1)
        return read_interface(self, id, offset, count, buffer);

    return 0;

}

static unsigned int write_interface(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct net_driver *driver = (struct net_driver *)self->driver;
    struct net_interface *interface = driver->interfaces[id - 2];

    return interface->write(interface, offset, count, buffer);

}

static unsigned int write(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id > 1)
        return write_interface(self, id, offset, count, buffer);

    return 0;

}

static unsigned int walk(struct modules_filesystem *self, unsigned int id, unsigned int count, void *buffer)
{

    if (!count)
        return 1;

    if (memory_match(buffer, "eth0", 4))
        return 2;

    return 0;

}

void net_filesystem_init(struct modules_filesystem *filesystem, struct modules_driver *driver)
{

    memory_clear(filesystem, sizeof (struct modules_filesystem));

    modules_filesystem_init(filesystem, 0x1001, driver, "net", 0, 0, read, write, 0, walk, 0); 

}

