#include <memory.h>
#include <string.h>
#include <vfs.h>
#include <modules/modules.h>
#include <video/video.h>

static unsigned int read_root(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_filesystem *filesystem = (struct video_filesystem *)self;
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

static unsigned int read_interface(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    memory_copy(buffer, "data\n", 5);

    return 5;

}

static unsigned int read_interface_data(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_filesystem *filesystem = (struct video_filesystem *)self;
    struct video_interface *interface = filesystem->interfaces[id];

    return interface->read(interface, offset, count, buffer);

}

static unsigned int read(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id >= 11000)
        return read_interface_data(self, id - 11000, offset, count, buffer);

    if (id >= 1000)
        return read_interface(self, id - 1000, offset, count, buffer);

    if (id == 1)
        return read_root(self, id, offset, count, buffer);

    return 0;

}

static unsigned int write_interface_data(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_filesystem *filesystem = (struct video_filesystem *)self;
    struct video_interface *interface = filesystem->interfaces[id];

    return interface->write(interface, offset, count, buffer);

}

static unsigned int write(struct vfs_filesystem *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id >= 21000)
        return 0;

    if (id >= 11000)
        return write_interface_data(self, id - 11000, offset, count, buffer);

    return 0;

}

static unsigned int walk_interface(struct vfs_filesystem *self, unsigned int id, unsigned int count, char *path)
{

    if (!count)
        return id;

    if (memory_match(path, "data", 4))
        return id + 10000;

    return 0;

}

static unsigned int walk(struct vfs_filesystem *self, unsigned int id, unsigned int count, char *path)
{

    if (!count)
        return id;

    if (memory_match(path, "0/", 2))
        return walk_interface(self, 1000, count - 2, path + 2);

    return 0;

}

static void register_interface(struct video_filesystem *self, struct video_interface *interface, struct modules_driver *driver, unsigned int (*read)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct video_interface *self, unsigned int offset, unsigned int count, void *buffer))
{

    interface->driver = driver;
    interface->read = read;
    interface->write = write;

    self->interfaces[self->interfacesCount] = interface;
    self->interfacesCount++;

}

static void register_protocol(struct video_filesystem *self, struct video_protocol *protocol, char *name)
{

    protocol->name = name;

    self->protocols[self->protocolsCount] = protocol;
    self->protocolsCount++;

}

static void unregister_interface(struct video_filesystem *self, struct video_interface *interface)
{

    self->interfacesCount--;

}

static void unregister_protocol(struct video_filesystem *self, struct video_protocol *protocol)
{

    self->protocolsCount--;

}

void video_filesystem_init(struct video_filesystem *filesystem)
{

    memory_clear(filesystem, sizeof (struct video_filesystem));

    vfs_filesystem_init(&filesystem->base, 1, "video", 0, 0, read, write, walk, 0);

    filesystem->register_interface = register_interface;
    filesystem->register_protocol = register_protocol;
    filesystem->unregister_interface = unregister_interface;
    filesystem->unregister_protocol = unregister_protocol;

}

