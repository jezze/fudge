#include <memory.h>
#include <string.h>
#include <vfs.h>
#include <base/base.h>
#include <arch/x86/vga/vga.h>
#include <tty/tty.h>

static unsigned int write_stdout(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct tty_filesystem *filesystem = (struct tty_filesystem *)self;
    char *stream = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
        filesystem->driver->putc(filesystem->driver, stream[i]);

    vga_set_cursor_offset(filesystem->driver->cursorOffset);

    return count;

}

static unsigned int read_cwd(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct tty_filesystem *filesystem = (struct tty_filesystem *)self;
    unsigned int length = string_length(filesystem->driver->cwdname);

    if (offset > length)
        return 0;

    if (count > length - offset)
        count = length - offset;

    memory_copy(buffer, filesystem->driver->cwdname + offset, count);

    return count;

}

static unsigned int write_cwd(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    struct tty_filesystem *filesystem = (struct tty_filesystem *)self;

    memory_clear(filesystem->driver->cwdname, TTY_CWD_SIZE);
    memory_copy(filesystem->driver->cwdname, buffer, count);

    return count;

}

static unsigned int read_root(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (offset > 17)
        return 0;

    if (count > 17 - offset)
        count = 17 - offset;

    memory_copy(buffer, "stdin\nstdout\ncwd\n" + offset, count);

    return count;

}

static unsigned int read(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id == 1)
        return read_root(self, id, offset, count, buffer);

    if (id == 4)
        return read_cwd(self, id, offset, count, buffer);

    return 0;

}

static unsigned int write(struct vfs_interface *self, unsigned int id, unsigned int offset, unsigned int count, void *buffer)
{

    if (id == 3)
        return write_stdout(self, id, offset, count, buffer);

    if (id == 4)
        return write_cwd(self, id, offset, count, buffer);

    return 0;

}

static unsigned int walk(struct vfs_interface *self, unsigned int id, unsigned int count, char *path)
{

    if (!count)
        return id;

    if (memory_match(path, "stdout", 6))
        return 3;

    if (memory_match(path, "cwd", 3))
        return 4;

    return 0;

}

void tty_filesystem_init(struct tty_filesystem *filesystem, struct tty_driver *driver)
{

    memory_clear(filesystem, sizeof (struct tty_filesystem));

    vfs_interface_init(&filesystem->base, 1, "tty", 0, 0, read, write, walk, 0);

    filesystem->driver = driver;

}

