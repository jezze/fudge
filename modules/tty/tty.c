#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <modules/vga/vga.h>
#include <modules/ps2/ps2.h>
#include <modules/tty/tty.h>

static struct tty_device device;

static void clear()
{

    char c = ' ';
    int i;

    for (i = 0; i < TTY_CHARACTER_SIZE; i++)
        device.vgaDevice->write_framebuffer(device.vgaDevice, i, 1, &c);

}

static void scroll()
{

    char buffer[TTY_CHARACTER_SIZE];

    device.vgaDevice->read_framebuffer(device.vgaDevice, TTY_CHARACTER_WIDTH, TTY_CHARACTER_SIZE - TTY_CHARACTER_WIDTH, buffer);

    unsigned int i;

    for (i = TTY_CHARACTER_SIZE - TTY_CHARACTER_WIDTH; i < TTY_CHARACTER_SIZE; i++)
        buffer[i] = ' ';

    device.vgaDevice->write_framebuffer(device.vgaDevice, 0, TTY_CHARACTER_SIZE, buffer);

    device.cursorOffset -= TTY_CHARACTER_WIDTH;

}

static void putc(char c)
{

    if (c == '\b')
    {

        device.cursorOffset--;

    }

    else if (c == '\t')
    {

        device.cursorOffset = (device.cursorOffset + 8) & ~(8 - 1);

    }

    else if (c == '\r')
    {

        device.cursorOffset -= (device.cursorOffset % TTY_CHARACTER_WIDTH);

    }

    else if (c == '\n')
    {

        device.cursorOffset += TTY_CHARACTER_WIDTH - (device.cursorOffset % TTY_CHARACTER_WIDTH);

    }
    
    else if (c >= ' ')
    {

        device.vgaDevice->write_framebuffer(device.vgaDevice, device.cursorOffset, 1, &c);
        device.cursorOffset++;

    }

    if (device.cursorOffset >= TTY_CHARACTER_WIDTH * TTY_CHARACTER_HEIGHT)
        scroll();

}

static unsigned int tty_device_cwd_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    count = string_length(device.cwdname);

    string_write(buffer, device.cwdname);

    return count;

}

static unsigned int tty_device_cwd_write(struct vfs_node *self, unsigned int count, void *buffer)
{

    count = string_length(buffer);

    string_write(device.cwdname, buffer);

    return count;

}

static unsigned int tty_device_in_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        char c;

        if (!device.kbdDevice->buffer.getc(&device.kbdDevice->buffer, &c))
            break;

        ((char *)buffer)[i] = c;

    }

    return i;

}

static unsigned int tty_device_out_write(struct vfs_node *self, unsigned int count, void *buffer)
{

    unsigned int i;

    for (i = 0; i < count; i++)
        putc(((char *)buffer)[i]);

    device.vgaDevice->set_cursor_offset(device.vgaDevice, device.cursorOffset);

    return count;

}

static unsigned int tty_device_view_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    void *out = buffer;
    unsigned int i;

    for (i = 0; i < VFS_FILESYSTEM_SLOTS; i++)
    {

        struct vfs_filesystem *filesystem = vfs_get_filesystem(i);

        if (!filesystem)
            continue;

        struct vfs_view *view = filesystem->find_view(filesystem, device.cwdname);

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
        return &device.nin;

    if (!string_compare("stdout", name))
        return &device.nout;

    if (!string_compare("stderr", name))
        return &device.nerror;

    if (!string_compare("cwd", name))
        return &device.ncwd;

    if (!string_compare("pwd", name))
        return &device.npwd;

    return 0;

}

static struct vfs_node *tty_device_view_walk(struct vfs_view *self, unsigned int index)
{

    if (index == 0)
        return &device.nin;

    if (index == 1)
        return &device.nout;

    if (index == 2)
        return &device.nerror;

    if (index == 3)
        return &device.ncwd;

    if (index == 4)
        return &device.npwd;

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


void tty_device_init(struct tty_device *device, char *cwdname)
{

    modules_device_init(&device->base, TTY_DEVICE_TYPE);
    device->cursorOffset = 0;
    device->kbdDevice = (struct kbd_device *)modules_get_device(KBD_DEVICE_TYPE, 0);
    device->vgaDevice = (struct vga_device *)modules_get_device(VGA_DEVICE_TYPE, 0);
    device->vgaDevice->set_cursor_color(device->vgaDevice, TTY_COLOR_WHITE, TTY_COLOR_BLACK);
    clear();

    string_write(device->cwdname, cwdname);

    vfs_node_init(&device->nin, 0, 0, 0, tty_device_in_read, 0);
    vfs_node_init(&device->nout, 1, 0, 0, 0, tty_device_out_write);
    vfs_node_init(&device->nerror, 2, 0, 0, 0, tty_device_out_write);
    vfs_node_init(&device->ncwd, 3, 0, 0, tty_device_cwd_read, tty_device_cwd_write);
    vfs_node_init(&device->npwd, 4, 0, 0, tty_device_view_read, 0);
    vfs_view_init(&device->nview, "tty", tty_device_view_find_node, tty_device_view_walk, tty_device_view_get_name);

    device->base.module.view = &device->nview;

}

void init()
{

    tty_device_init(&device, "home");

    modules_register_device(&device.base);

}

void destroy()
{

    modules_unregister_device(&device.base);

}

