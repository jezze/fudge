#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/vga/vga.h>
#include <modules/ps2/ps2.h>
#include <modules/tty/tty.h>

static struct tty_driver *driver;
static struct vfs_view ttyView;
static struct vfs_node ttyNodes[5];

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

        struct vfs_view *view = filesystem->find_view(filesystem, driver->cwdname);

        if (!view)
            continue;

        unsigned int j;
        unsigned int start = 0;

        for (j = 0; j < 64; j++)
        {

            struct vfs_node *node = view->walk(view, j);

            if (!node)
                continue;

            string_write_format(buffer, "%s\n", node->name);
            buffer += string_length(buffer);

        }

    }

    return string_length(out);

}

static struct vfs_node *view_find_node(struct vfs_view *self, char *name)
{

    unsigned int i;

    for (i = 0; i < 5; i++)
    {

        if (!string_compare(ttyNodes[i].name, name))
            return &ttyNodes[i];

    }

    return 0;

}

static struct vfs_node *view_walk(struct vfs_view *self, unsigned int index)
{

    if (index > 4)
        return 0;

    return &ttyNodes[index];

}

void tty_view_init(struct modules_module *module)
{

    driver = (struct tty_driver *)module;

    vfs_node_init(&ttyNodes[0], "stdin", 0, 0, 0, in_read, 0);
    vfs_node_init(&ttyNodes[1], "stdout", 1, 0, 0, 0, out_write);
    vfs_node_init(&ttyNodes[2], "stderr", 2, 0, 0, 0, out_write);
    vfs_node_init(&ttyNodes[3], "cwd", 3, 0, 0, cwd_read, cwd_write);
    vfs_node_init(&ttyNodes[4], "pwd", 4, 0, 0, pwd_read, 0);
    vfs_view_init(&ttyView, "tty", view_find_node, view_walk);

    module->view = &ttyView;

}

