#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/vga/vga.h>
#include <modules/ps2/ps2.h>
#include <modules/tty/tty.h>

static struct vfs_view nview;
static struct vfs_node nin;
static struct vfs_node nout;
static struct vfs_node nerror;
static struct vfs_node ncwd;
static struct vfs_node npwd;

static unsigned int tty_device_cwd_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    struct tty_device *device = tty_get();

    count = string_length(device->cwdname);

    string_write(buffer, device->cwdname);

    return count;

}

static unsigned int tty_device_cwd_write(struct vfs_node *self, unsigned int count, void *buffer)
{

    struct tty_device *device = tty_get();

    count = string_length(buffer);

    string_write(device->cwdname, buffer);

    return count;

}

static unsigned int tty_device_in_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    struct tty_device *device = tty_get();

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        char c;

        if (!device->kbdDevice->buffer.getc(&device->kbdDevice->buffer, &c))
            break;

        ((char *)buffer)[i] = c;

    }

    return i;

}

static unsigned int tty_device_out_write(struct vfs_node *self, unsigned int count, void *buffer)
{

    struct tty_device *device = tty_get();

    unsigned int i;

    for (i = 0; i < count; i++)
        device->putc(device, ((char *)buffer)[i]);

    device->vgaDevice->set_cursor_offset(device->vgaDevice, device->cursorOffset);

    return count;

}

static unsigned int tty_device_view_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    struct tty_device *device = tty_get();

    void *out = buffer;
    unsigned int i;

    for (i = 0; i < VFS_FILESYSTEM_SLOTS; i++)
    {

        struct vfs_filesystem *filesystem = vfs_get_filesystem(i);

        if (!filesystem)
            continue;

        struct vfs_view *view = filesystem->find_view(filesystem, device->cwdname);

        if (!view)
            continue;

        unsigned int j;
        unsigned int start = 0;

        for (j = 0; j < 64; j++)
        {

            struct vfs_node *node = view->walk(view, j);

            if (!node)
                continue;

            string_write_format(buffer, "%s\n", view->get_name(view, node));
            buffer += string_length(buffer);

        }

    }

    return string_length(out);

}

static struct vfs_node *tty_device_view_find_node(struct vfs_view *self, char *name)
{

    if (!string_compare("stdin", name))
        return &nin;

    if (!string_compare("stdout", name))
        return &nout;

    if (!string_compare("stderr", name))
        return &nerror;

    if (!string_compare("cwd", name))
        return &ncwd;

    if (!string_compare("pwd", name))
        return &npwd;

    return 0;

}

static struct vfs_node *tty_device_view_walk(struct vfs_view *self, unsigned int index)
{

    if (index == 0)
        return &nin;

    if (index == 1)
        return &nout;

    if (index == 2)
        return &nerror;

    if (index == 3)
        return &ncwd;

    if (index == 4)
        return &npwd;

    return 0;

}

static char *tty_device_view_get_name(struct vfs_view *self, struct vfs_node *node)
{

    if (node->id == 0)
        return "stdin";

    if (node->id == 1)
        return "stdout";
 
    if (node->id == 2)
        return "stderr";

    if (node->id == 3)
        return "cwd";

    if (node->id == 4)
        return "pwd";

    return 0;

}

void tty_view_init(struct tty_device *tty)
{

    vfs_node_init(&nin, 0, 0, 0, tty_device_in_read, 0);
    vfs_node_init(&nout, 1, 0, 0, 0, tty_device_out_write);
    vfs_node_init(&nerror, 2, 0, 0, 0, tty_device_out_write);
    vfs_node_init(&ncwd, 3, 0, 0, tty_device_cwd_read, tty_device_cwd_write);
    vfs_node_init(&npwd, 4, 0, 0, tty_device_view_read, 0);
    vfs_view_init(&nview, "tty", tty_device_view_find_node, tty_device_view_walk, tty_device_view_get_name);

    tty->base.module.view = &nview;

}

