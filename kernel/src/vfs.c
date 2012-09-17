#include <memory.h>
#include <vfs.h>

unsigned int vfs_read8(char value, unsigned int offset, unsigned int count, void *buffer)
{

    if (offset >= 1)
        return 0;

    if (count + offset >= 1)
        count = 1 - offset;

    memory_copy(((char *)&value) + offset, buffer, count);

    return count;

}


unsigned int vfs_read16(short value, unsigned int offset, unsigned int count, void *buffer)
{

    if (offset >= 2)
        return 0;

    if (count + offset >= 2)
        count = 2 - offset;

    memory_copy(((char *)&value) + offset, buffer, count);

    return count;

}

unsigned int vfs_read32(int value, unsigned int offset, unsigned int count, void *buffer)
{

    if (offset >= 4)
        return 0;

    if (count + offset >= 4)
        count = 4 - offset;

    memory_copy(((char *)&value) + offset, buffer, count);

    return count;

}

unsigned int vfs_write8(char value, unsigned int offset, unsigned int count, void *buffer)
{

    if (offset >= 1)
        return 0;

    if (count + offset >= 1)
        count = 1 - offset;

    memory_copy(buffer, ((char *)&value) + offset, count);

    return count;

}

unsigned int vfs_write16(short value, unsigned int offset, unsigned int count, void *buffer)
{

    if (offset >= 2)
        return 0;

    if (count + offset >= 2)
        count = 2 - offset;

    memory_copy(buffer, ((char *)&value) + offset, count);

    return count;

}

unsigned int vfs_write32(int value, unsigned int offset, unsigned int count, void *buffer)
{

    if (offset >= 4)
        return 0;

    if (count + offset >= 4)
        count = 4 - offset;

    memory_copy(buffer, ((char *)&value) + offset, count);

    return count;

}

void vfs_init_interface(struct vfs_interface *interface, unsigned int rootid, char *name, void (*open)(struct vfs_interface *self, unsigned int id), void (*close)(struct vfs_interface *self, unsigned int id), unsigned int (*read)(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*parent)(struct vfs_interface *self, unsigned int id), unsigned int (*walk)(struct vfs_interface *self, unsigned int id, unsigned int count, char *path), unsigned int (*get_physical)(struct vfs_interface *self, unsigned int id))
{

    memory_clear(interface, sizeof (struct vfs_interface));

    interface->rootid = rootid;
    interface->open = open;
    interface->close = close;
    interface->read = read;
    interface->write = write;
    interface->parent = parent;
    interface->walk = walk;
    interface->get_physical = get_physical;

}

