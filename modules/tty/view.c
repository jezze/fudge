#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/vga/vga.h>
#include <modules/ps2/ps2.h>
#include <modules/tty/tty.h>

static struct vfs_view ttyView;
static struct vfs_node ttyIn;
static struct vfs_node ttyOut;
static struct vfs_node ttyError;
static struct vfs_node ttyCwd;
static struct vfs_node ttyPwd;

static unsigned int cwd_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    struct tty_device *device = tty_get();

    count = string_length(device->cwdname);

    string_write(buffer, device->cwdname);

    return count;

}

static unsigned int cwd_write(struct vfs_node *self, unsigned int count, void *buffer)
{

    struct tty_device *device = tty_get();

    count = string_length(buffer);

    string_write(device->cwdname, buffer);

    return count;

}

static unsigned int in_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    struct tty_device *device = tty_get();

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        char c;

        if (!device->kbdDriver->buffer.getc(&device->kbdDriver->buffer, &c))
            break;

        ((char *)buffer)[i] = c;

    }

    return i;

}

static unsigned int out_write(struct vfs_node *self, unsigned int count, void *buffer)
{

    struct tty_device *device = tty_get();

    unsigned int i;

    for (i = 0; i < count; i++)
        device->putc(device, ((char *)buffer)[i]);

    device->vgaDevice->set_cursor_offset(device->vgaDevice, device->cursorOffset);

    return count;

}

static unsigned int pwd_read(struct vfs_node *self, unsigned int count, void *buffer)
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

static struct vfs_node *view_find_node(struct vfs_view *self, char *name)
{

    if (!string_compare("stdin", name))
        return &ttyIn;

    if (!string_compare("stdout", name))
        return &ttyOut;

    if (!string_compare("stderr", name))
        return &ttyError;

    if (!string_compare("cwd", name))
        return &ttyCwd;

    if (!string_compare("pwd", name))
        return &ttyPwd;

    return 0;

}

static struct vfs_node *view_walk(struct vfs_view *self, unsigned int index)
{

    if (index == 0)
        return &ttyIn;

    if (index == 1)
        return &ttyOut;

    if (index == 2)
        return &ttyError;

    if (index == 3)
        return &ttyCwd;

    if (index == 4)
        return &ttyPwd;

    return 0;

}

static char *view_get_name(struct vfs_view *self, struct vfs_node *node)
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

struct vfs_view *tty_view_init()
{

    vfs_node_init(&ttyIn, 0, 0, 0, in_read, 0);
    vfs_node_init(&ttyOut, 1, 0, 0, 0, out_write);
    vfs_node_init(&ttyError, 2, 0, 0, 0, out_write);
    vfs_node_init(&ttyCwd, 3, 0, 0, cwd_read, cwd_write);
    vfs_node_init(&ttyPwd, 4, 0, 0, pwd_read, 0);
    vfs_view_init(&ttyView, "tty", view_find_node, view_walk, view_get_name);

    return &ttyView;

}

