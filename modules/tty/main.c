#include <lib/string.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/nodefs/nodefs.h>
#include <modules/ps2/ps2.h>
#include <modules/vga/vga.h>
#include <modules/tty/tty.h>

static struct tty_driver driver;
static struct vfs_node in;
static struct vfs_node out;
static struct vfs_node err;
static struct vfs_node cwd;
static struct vfs_node pwd;

static unsigned int cwd_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    count = string_length(driver.cwdname);

    string_write(buffer, driver.cwdname);

    return count;

}

static unsigned int cwd_write(struct vfs_node *self, unsigned int count, void *buffer)
{

    count = string_length(buffer);

    string_write(driver.cwdname, buffer);

    return count;

}

static unsigned int in_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        char c;

        if (!driver.kbdDriver->buffer.getc(&driver.kbdDriver->buffer, &c))
            break;

        ((char *)buffer)[i] = c;

    }

    return i;

}

static unsigned int out_write(struct vfs_node *self, unsigned int count, void *buffer)
{

    unsigned int i;

    for (i = 0; i < count; i++)
        driver.putc(&driver, ((char *)buffer)[i]);

    driver.vgaDevice->set_cursor_offset(driver.vgaDevice, driver.cursorOffset);

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

        unsigned int index = filesystem->firstIndex;

        while ((index = filesystem->walk(filesystem, index)))
        {

            struct vfs_node *node = filesystem->get_node(filesystem, index - 1);

            if (!node)
                continue;

            if (string_compare(driver.cwdname, "*") && !string_find(node->name, driver.cwdname))
                continue;

            string_write_format(buffer, "%d\t%s\n", index, node->name);
            buffer += string_length(buffer);

        }

    }

    return string_length(out);

}

void init()
{

    tty_driver_init(&driver, "home");
    modules_register_driver(&driver.base);

    struct nodefs_driver *nodefs = (struct nodefs_driver *)modules_get_driver(NODEFS_DRIVER_TYPE);

    if (!nodefs)
        return;

    vfs_node_init(&in, "module/tty/stdin", 0, 0, 0, in_read, 0);
    vfs_node_init(&out, "module/tty/stdout", 1, 0, 0, 0, out_write);
    vfs_node_init(&err, "module/tty/stderr", 2, 0, 0, 0, out_write);
    vfs_node_init(&cwd, "module/tty/cwd", 3, 0, 0, cwd_read, cwd_write);
    vfs_node_init(&pwd, "module/tty/pwd", 4, 0, 0, pwd_read, 0);

    nodefs->register_node(nodefs, &in);
    nodefs->register_node(nodefs, &out);
    nodefs->register_node(nodefs, &err);
    nodefs->register_node(nodefs, &cwd);
    nodefs->register_node(nodefs, &pwd);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

