#include <memory.h>
#include <vfs.h>

unsigned int vfs_read(void *out, unsigned int ocount, void *in, unsigned int icount, unsigned int offset)
{

    char *op = out;
    const char *ip = in;
    unsigned int count;

    if (offset >= icount)
        return 0;

    if (ocount > icount - offset)
        ocount = icount - offset;

    ip += offset;

    for (count = ocount; count; count--)
        *op++ = *ip++;

    return ocount;

}

unsigned int vfs_write(void *out, unsigned int ocount, void *in, unsigned int icount, unsigned int offset)
{

    char *op = out;
    const char *ip = in;
    unsigned int count;

    if (offset >= ocount)
        return 0;

    if (icount > ocount - offset)
        icount = ocount - offset;

    op += offset;

    for (count = icount; count; count--)
        *op++ = *ip++;

    return icount;

}

void vfs_init_interface(struct vfs_interface *interface, unsigned int rootid, char *name, void (*open)(struct vfs_interface *self, unsigned int id), void (*close)(struct vfs_interface *self, unsigned int id), unsigned int (*read)(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*walk)(struct vfs_interface *self, unsigned int id, unsigned int count, char *path), unsigned int (*get_physical)(struct vfs_interface *self, unsigned int id))
{

    memory_clear(interface, sizeof (struct vfs_interface));

    interface->rootid = rootid;
    interface->open = open;
    interface->close = close;
    interface->read = read;
    interface->write = write;
    interface->walk = walk;
    interface->get_physical = get_physical;

}

