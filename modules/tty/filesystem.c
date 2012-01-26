#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/vga/vga.h>
#include <modules/ps2/ps2.h>
#include <modules/tty/tty.h>

static struct tty_driver *driver;
static struct vfs_node ttyNodes[5];
static struct vfs_filesystem filesystem;

static unsigned int cwd_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    count = string_length(driver->cwdname);

    string_write(buffer, driver->cwdname);

    return count;

}

static unsigned int cwd_write(struct vfs_node *self, unsigned int count, void *buffer)
{

    count = string_length(buffer);

    string_write(driver->cwdname, buffer);

    return count;

}

static unsigned int in_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        char c;

        if (!driver->kbdDriver->buffer.getc(&driver->kbdDriver->buffer, &c))
            break;

        ((char *)buffer)[i] = c;

    }

    return i;

}

static unsigned int out_write(struct vfs_node *self, unsigned int count, void *buffer)
{

    unsigned int i;

    for (i = 0; i < count; i++)
        driver->putc(driver, ((char *)buffer)[i]);

    driver->vgaDevice->set_cursor_offset(driver->vgaDevice, driver->cursorOffset);

    return count;

}

static unsigned int pwd_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    void *out = buffer;
    unsigned int i;

    for (i = 0; i < VFS_FILESYSTEM_SLOTS; i++)
    {

        struct vfs_filesystem *filesystem = vfs_get_filesystem(i);

        if (!filesystem)
            continue;

        unsigned int index = 0;

        while ((index = filesystem->walk(filesystem, index)))
        {

            struct vfs_node *node = filesystem->get_node(filesystem, index);

            if (!node)
                continue;

            if (string_compare(driver->cwdname, "*") && !string_find(node->name, driver->cwdname))
                continue;

            string_write_format(buffer, "%s\n", node->name);
            buffer += string_length(buffer);

        }

    }

    return string_length(out);

}

static struct vfs_node *filesystem_get_node(struct vfs_filesystem *self, unsigned int index)
{

    if (index >= self->nodeCount)
        return 0;

    return &ttyNodes[index];

}

static struct vfs_node *filesystem_find_node(struct vfs_filesystem *self, char *name)
{

    unsigned int i;

    for (i = 0; i < self->nodeCount; i++)
    {

        if (string_find(ttyNodes[i].name, name))
            return &ttyNodes[i];

    }

    return 0;

}

static unsigned int filesystem_walk(struct vfs_filesystem *self, unsigned int index)
{

    if (index >= self->nodeCount)
        return 0;

    return index + 1;

}

void tty_filesystem_init(struct modules_module *module)
{

    driver = (struct tty_driver *)module;

    vfs_node_init(&ttyNodes[0], "module/tty/stdin", 0, 0, 0, in_read, 0);
    vfs_node_init(&ttyNodes[1], "module/tty/stdout", 1, 0, 0, 0, out_write);
    vfs_node_init(&ttyNodes[2], "module/tty/stderr", 2, 0, 0, 0, out_write);
    vfs_node_init(&ttyNodes[3], "module/tty/cwd", 3, 0, 0, cwd_read, cwd_write);
    vfs_node_init(&ttyNodes[4], "module/tty/pwd", 4, 0, 0, pwd_read, 0);

    vfs_filesystem_init(&filesystem, filesystem_get_node, filesystem_find_node, filesystem_walk); 
    filesystem.nodeCount = 5;

    vfs_register_filesystem(&filesystem);

}

