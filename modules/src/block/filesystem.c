#include <memory.h>
#include <string.h>
#include <modules.h>
#include <block/block.h>

static unsigned int read_root(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct block_driver *driver = (struct block_driver *)self->driver;
    unsigned int i;
    unsigned int c = 0;

    for (i = 0; i < driver->interfacesCount; i++)
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

static unsigned int read_interface(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    memory_copy(buffer, "data\n", 5);

    return 9;

}

static unsigned int read_interface_data(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct block_driver *driver = (struct block_driver *)self->driver;
    struct block_interface *interface = driver->interfaces[id];

    return interface->read(interface, offset, count, buffer);

}

static unsigned int read(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id >= 11000)
        return read_interface_data(self, id - 11000, offset, count, buffer);

    if (id >= 1000)
        return read_interface(self, id - 1000, offset, count, buffer);

    if (id == 1)
        return read_root(self, id, offset, count, buffer);

    return 0;

}

static unsigned int write_interface_data(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct block_driver *driver = (struct block_driver *)self->driver;
    struct block_interface *interface = driver->interfaces[id];

    return interface->write(interface, offset, count, buffer);

}

static unsigned int write(struct modules_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id >= 21000)
        return 0;

    if (id >= 11000)
        return write_interface_data(self, id - 11000, offset, count, buffer);

    return 0;

}

static unsigned int walk_interface(struct modules_filesystem *self, unsigned int id, unsigned int count, char *path)
{

    if (!count)
        return id;

    if (memory_match(path, "data", 4))
        return id + 10000;

    return 0;

}

static unsigned int walk(struct modules_filesystem *self, unsigned int id, unsigned int count, char *path)
{

    if (!count)
        return id;

    if (memory_match(path, "0/", 2))
        return walk_interface(self, 1000, count - 2, path + 2);

    return 0;

}

void block_filesystem_init(struct modules_filesystem *filesystem, struct modules_driver *driver)
{

    memory_clear(filesystem, sizeof (struct modules_filesystem));

    modules_filesystem_init(filesystem, 0x1001, driver, 1, "block", 0, 0, read, write, walk, 0); 

}

