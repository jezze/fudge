#include <lib/string.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/nodefs/nodefs.h>
#include <modules/ps2/ps2.h>
#include <modules/vga/vga.h>
#include <modules/tty/tty.h>

static struct tty_driver driver;
static struct nodefs_node in;
static struct nodefs_node out;
static struct nodefs_node err;
static struct nodefs_node cwd;
static struct nodefs_node pwd;

static unsigned int cwd_read(struct nodefs_node *self, unsigned int count, void *buffer)
{

    count = string_length(driver.cwdname);

    string_write(buffer, driver.cwdname);

    return count;

}

static unsigned int cwd_write(struct nodefs_node *self, unsigned int count, void *buffer)
{

    count = string_length(buffer);

    string_write(driver.cwdname, buffer);

    return count;

}

static unsigned int in_read(struct nodefs_node *self, unsigned int count, void *buffer)
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

static unsigned int out_write(struct nodefs_node *self, unsigned int count, void *buffer)
{

    unsigned int i;

    for (i = 0; i < count; i++)
        driver.putc(&driver, ((char *)buffer)[i]);

    driver.vgaDriver->set_cursor_offset(driver.vgaDriver, driver.cursorOffset);

    return count;

}

static unsigned int pwd_read(struct nodefs_node *self, unsigned int count, void *buffer)
{

    void *out = buffer;
    unsigned int i;

    for (i = 0; i < VFS_FILESYSTEM_SLOTS; i++)
    {

        struct vfs_filesystem *filesystem = vfs_get_filesystem(i);

        if (!filesystem)
            continue;

        unsigned int index = filesystem->firstId;

        do
        {

            char *name = filesystem->get_name(filesystem, index);

            if (string_compare(driver.cwdname, "*") && !string_find(name, driver.cwdname))
                continue;

            string_write_format(buffer, "%d\t%s%s\n", index, filesystem->name, name);
            buffer += string_length(buffer);

        }
        while ((index = filesystem->walk(filesystem, index)));

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

    in.name = "tty/stdin";
    in.read = in_read;
    in.write = 0;

    out.name = "tty/stdout";
    out.read = 0;
    out.write = out_write;

    err.name = "tty/stderr";
    err.read = 0;
    err.write = out_write;

    cwd.name = "tty/cwd";
    cwd.read = cwd_read;
    cwd.write = cwd_write;

    pwd.name = "tty/pwd";
    pwd.read = pwd_read;
    pwd.write = 0;

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

