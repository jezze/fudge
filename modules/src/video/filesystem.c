#include <memory.h>
#include <string.h>
#include <vfs.h>
#include <modules/modules.h>
#include <video/video.h>

static unsigned int read_root(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
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

static unsigned int read_interface(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    memory_copy(buffer, "bpp\ndata\n", 9);

    return 9;

}

static unsigned int read_interface_bpp(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_filesystem *filesystem = (struct video_filesystem *)self;
    struct video_interface *interface = filesystem->interfaces[id];

    unsigned int bpp = interface->read_bpp(interface);

    memory_copy(buffer, &bpp, 4);

    return 4;

}

static unsigned int read_interface_data(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_filesystem *filesystem = (struct video_filesystem *)self;
    struct video_interface *interface = filesystem->interfaces[id];

    return interface->read_data(interface, offset, count, buffer);

}

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id >= 21000)
        return read_interface_bpp(self, id - 21000, offset, count, buffer);

    if (id >= 11000)
        return read_interface_data(self, id - 11000, offset, count, buffer);

    if (id >= 1000)
        return read_interface(self, id - 1000, offset, count, buffer);

    if (id == 1)
        return read_root(self, id, offset, count, buffer);

    return 0;

}

static unsigned int write_interface_bpp(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_filesystem *filesystem = (struct video_filesystem *)self;
    struct video_interface *interface = filesystem->interfaces[id];
    unsigned int bpp = 0;

    memory_copy(&bpp, buffer, 4);

    interface->write_bpp(interface, bpp);

    return 4;

}

static unsigned int write_interface_data(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_filesystem *filesystem = (struct video_filesystem *)self;
    struct video_interface *interface = filesystem->interfaces[id];

    return interface->write_data(interface, offset, count, buffer);

}

static unsigned int write(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id >= 21000)
        return write_interface_bpp(self, id - 21000, offset, count, buffer);;

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

    if (memory_match(path, "bpp", 4))
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

void video_filesystem_init(struct video_filesystem *filesystem)
{

    memory_clear(filesystem, sizeof (struct video_filesystem));

    vfs_interface_init(&filesystem->base, 1, "video", 0, 0, read, write, walk, 0);

}

