#include <memory.h>
#include <string.h>
#include <vfs.h>
#include <base/base.h>
#include <video/video.h>

static unsigned int read_root(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_filesystem *filesystem = (struct video_filesystem *)self;
    unsigned int i;
    unsigned int c = 0;

    if (offset > 0)
        return 0;

    memory_copy((char *)buffer + c, "../\n", 4);
    c += 4;

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

    if (offset > 0)
        return 0;

    memory_copy(buffer, "../\nbpp\ndata\nenable\nxres\nyres\n", 30);

    return 30;

}

static unsigned int read_interface_bpp(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int index = (id >> 8) & 0xFF;
    struct video_filesystem *filesystem = (struct video_filesystem *)self;
    struct video_interface *interface = filesystem->interfaces[index];

    memory_copy(buffer, &interface->bpp, 4);

    return 4;

}

static unsigned int read_interface_data(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int index = (id >> 8) & 0xFF;
    struct video_filesystem *filesystem = (struct video_filesystem *)self;
    struct video_interface *interface = filesystem->interfaces[index];

    return interface->read_data(interface, offset, count, buffer);

}

static unsigned int read_interface_enable(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int read_interface_xres(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int index = (id >> 8) & 0xFF;
    struct video_filesystem *filesystem = (struct video_filesystem *)self;
    struct video_interface *interface = filesystem->interfaces[index];

    memory_copy(buffer, &interface->xres, 4);

    return 4;

}

static unsigned int read_interface_yres(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int index = (id >> 8) & 0xFF;
    struct video_filesystem *filesystem = (struct video_filesystem *)self;
    struct video_interface *interface = filesystem->interfaces[index];

    memory_copy(buffer, &interface->yres, 4);

    return 4;

}

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int type = id & 0xFF;

    if (id >= 0x00010000)
    {

        if (type == 0)
            return read_interface_data(self, id, offset, count, buffer);

        if (type == 1)
            return read_interface_bpp(self, id, offset, count, buffer);;

        if (type == 2)
            return read_interface_xres(self, id, offset, count, buffer);;

        if (type == 3)
            return read_interface_yres(self, id, offset, count, buffer);;

        if (type == 4)
            return read_interface_enable(self, id, offset, count, buffer);;

    }

    if (id >= 0x00000100)
        return read_interface(self, id, offset, count, buffer);

    if (id == 0x00000001)
        return read_root(self, id, offset, count, buffer);

    return 0;

}

static unsigned int write_interface_bpp(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int index = (id >> 8) & 0xFF;
    struct video_filesystem *filesystem = (struct video_filesystem *)self;
    struct video_interface *interface = filesystem->interfaces[index];

    memory_copy(&interface->bpp, buffer, 4);

    return 4;

}

static unsigned int write_interface_data(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int index = (id >> 8) & 0xFF;
    struct video_filesystem *filesystem = (struct video_filesystem *)self;
    struct video_interface *interface = filesystem->interfaces[index];

    return interface->write_data(interface, offset, count, buffer);

}

static unsigned int write_interface_enable(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int index = (id >> 8) & 0xFF;
    struct video_filesystem *filesystem = (struct video_filesystem *)self;
    struct video_interface *interface = filesystem->interfaces[index];

    interface->enable(interface);

    return 0;

}

static unsigned int write_interface_xres(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int index = (id >> 8) & 0xFF;
    struct video_filesystem *filesystem = (struct video_filesystem *)self;
    struct video_interface *interface = filesystem->interfaces[index];

    memory_copy(&interface->xres, buffer, 4);

    return 4;

}

static unsigned int write_interface_yres(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int index = (id >> 8) & 0xFF;
    struct video_filesystem *filesystem = (struct video_filesystem *)self;
    struct video_interface *interface = filesystem->interfaces[index];

    memory_copy(&interface->yres, buffer, 4);

    return 4;

}

static unsigned int write(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int type = id & 0xFF;

    if (id >= 0x00010000)
    {

        if (type == 0)
            return write_interface_data(self, id, offset, count, buffer);

        if (type == 1)
            return write_interface_bpp(self, id, offset, count, buffer);;

        if (type == 2)
            return write_interface_xres(self, id, offset, count, buffer);;

        if (type == 3)
            return write_interface_yres(self, id, offset, count, buffer);;

        if (type == 4)
            return write_interface_enable(self, id, offset, count, buffer);;

    }

    return 0;

}

static unsigned int walk(struct vfs_interface *self, unsigned int id, unsigned int count, char *path)
{

    if (!count)
        return id;

    if (id >= 0x00000100)
    {

        if (memory_match(path, "../", 3))
            return walk(self, id >> 8, count - 3, path + 3);

        if (memory_match(path, "data", 4))
            return walk(self, (id << 8) + 0, count - 4, path + 4);

        if (memory_match(path, "bpp", 3))
            return walk(self, (id << 8) + 1, count - 3, path + 3);

        if (memory_match(path, "xres", 4))
            return walk(self, (id << 8) + 2, count - 4, path + 4);

        if (memory_match(path, "yres", 4))
            return walk(self, (id << 8) + 3, count - 4, path + 4);

        if (memory_match(path, "enable", 6))
            return walk(self, (id << 8) + 4, count - 6, path + 6);

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

void video_filesystem_init(struct video_filesystem *filesystem)
{

    memory_clear(filesystem, sizeof (struct video_filesystem));

    vfs_init_interface(&filesystem->base, 1, "video", 0, 0, read, write, 0, walk, 0);

}

