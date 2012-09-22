#include <memory.h>
#include <string.h>
#include <vfs.h>
#include <base/base.h>
#include <video/video.h>

static unsigned int read_root(struct video_filesystem *filesystem, unsigned int offset, unsigned int count, void *buffer)
{

    char temp[1024];
    unsigned int o = 0;
    unsigned int c = 1024;
    unsigned int i;

    o += vfs_write(temp, c - o, "../\n", 4, o);

    for (i = 0; i < filesystem->interfacesCount; i++)
    {

        string_write_num(temp + o, i, 10);
        o += string_length(temp + o);
        o += vfs_write(temp, c - o, "/\n", 2, o);

    }

    return vfs_read(buffer, count, temp, o, offset);

}

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_filesystem *filesystem = (struct video_filesystem *)self;

    if (id >= 0x00010000)
    {

        unsigned int type = (id) & 0xFF;
        unsigned int index = (id >> 8) & 0xFF;
        struct video_interface *interface = filesystem->interfaces[index];

        if (type == 3)
            return vfs_read(buffer, count, &interface->yres, 4, offset);

        if (type == 2)
            return vfs_read(buffer, count, &interface->xres, 4, offset);

        if (type == 1)
            return vfs_read(buffer, count, &interface->bpp, 4, offset);

        if (type == 0)
            return interface->read_data(interface, offset, count, buffer);

    }

    if (id >= 0x00000100)
        return vfs_read(buffer, count, "../\nbpp\ndata\nenable\nxres\nyres\n", 30, offset);

    if (id == 0x00000001)
        return read_root(filesystem, offset, count, buffer);

    return 0;

}

static unsigned int write(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct video_filesystem *filesystem = (struct video_filesystem *)self;

    if (id >= 0x00010000)
    {

        unsigned int type = (id) & 0xFF;
        unsigned int index = (id >> 8) & 0xFF;
        struct video_interface *interface = filesystem->interfaces[index];

        if (type == 4)
        {

            interface->enable(interface);

            return 0;

        }

        if (type == 3)
            return vfs_write(&interface->yres, 4, buffer, count, offset);

        if (type == 2)
            return vfs_write(&interface->xres, 4, buffer, count, offset);

        if (type == 1)
            return vfs_write(&interface->bpp, 4, buffer, count, offset);

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

void video_init_filesystem(struct video_filesystem *filesystem)
{

    memory_clear(filesystem, sizeof (struct video_filesystem));

    vfs_init_interface(&filesystem->base, 1, "video", 0, 0, read, write, 0, walk, 0);

}

