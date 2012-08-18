#include <memory.h>
#include <string.h>
#include <modules.h>
#include <net/net.h>

static unsigned int read_root(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct net_filesystem *filesystem = (struct net_filesystem *)self;
    unsigned int i;
    unsigned int c = 0;

    for (i = 0; i < filesystem->driversCount; i++)
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

static unsigned int read_driver(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    memory_copy(buffer, "data\nmac\n", 9);

    return 9;

}

static unsigned int read_driver_data(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct net_filesystem *filesystem = (struct net_filesystem *)self;
    struct net_driver *driver = filesystem->drivers[id];

    return driver->read(driver, offset, count, buffer);

}

static unsigned int read_driver_mac(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct net_filesystem *filesystem = (struct net_filesystem *)self;
    struct net_driver *driver = filesystem->drivers[id];

    memory_copy(buffer, driver->mac, 6);

    return 6;

}

static unsigned int read(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id >= 21000)
        return read_driver_mac(self, id - 21000, offset, count, buffer);

    if (id >= 11000)
        return read_driver_data(self, id - 11000, offset, count, buffer);

    if (id >= 1000)
        return read_driver(self, id - 1000, offset, count, buffer);

    if (id == 1)
        return read_root(self, id, offset, count, buffer);

    return 0;

}

static unsigned int write_driver_data(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct net_filesystem *filesystem = (struct net_filesystem *)self;
    struct net_driver *driver = filesystem->drivers[id];

    return driver->write(driver, offset, count, buffer);

}

static unsigned int write(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id >= 21000)
        return 0;

    if (id >= 11000)
        return write_driver_data(self, id - 11000, offset, count, buffer);

    return 0;

}

static unsigned int walk_driver(struct modules_filesystem *self, unsigned int id, unsigned int count, char *path)
{

    if (!count)
        return id;

    if (memory_match(path, "data", 4))
        return id + 10000;

    if (memory_match(path, "mac", 3))
        return id + 20000;

    return 0;

}

static unsigned int walk(struct modules_filesystem *self, unsigned int id, unsigned int count, char *path)
{

    if (!count)
        return id;

    if (memory_match(path, "0/", 2))
        return walk_driver(self, 1000, count - 2, path + 2);

    return 0;

}

static void register_driver(struct net_filesystem *self, struct net_driver *driver, unsigned int (*read)(struct net_driver *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct net_driver *self, unsigned int offset, unsigned int count, void *buffer))
{

    driver->read = read;
    driver->write = write;

    self->drivers[self->driversCount] = driver;
    self->driversCount++;

}

static void register_protocol(struct net_filesystem *self, struct net_protocol *protocol, char *name)
{

    protocol->name = name;

    self->protocols[self->protocolsCount] = protocol;
    self->protocolsCount++;

}

static void unregister_driver(struct net_filesystem *self, struct net_driver *driver)
{

    self->driversCount--;

}

static void unregister_protocol(struct net_filesystem *self, struct net_protocol *protocol)
{

    self->protocolsCount--;

}

void net_filesystem_init(struct net_filesystem *filesystem)
{

    memory_clear(filesystem, sizeof (struct net_filesystem));

    modules_filesystem_init(&filesystem->base, 0x1001, 0, 1, "net", 0, 0, read, write, walk, 0); 

    filesystem->register_driver = register_driver;
    filesystem->register_protocol = register_protocol;
    filesystem->unregister_driver = unregister_driver;
    filesystem->unregister_protocol = unregister_protocol;

}

