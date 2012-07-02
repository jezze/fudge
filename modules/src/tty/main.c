#include <memory.h>
#include <string.h>
#include <modules.h>
#include <arch/x86/ps2/ps2.h>
#include <arch/x86/vga/vga.h>
#include <nodefs/nodefs.h>
#include <tty/tty.h>

static struct tty_driver driver;
static struct nodefs_node in;
static struct nodefs_node out;
static struct nodefs_node cwd;

static unsigned int in_read(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    char *stream = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (!ps2_getc(stream + i))
            break;

    }

    return i;

}

static unsigned int in_write(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    char *stream = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (!ps2_putc(stream + i))
            break;

    }

    return i;

}

static unsigned int out_write(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    char *stream = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
        driver.putc(&driver, stream[i]);

    vga_set_cursor_offset(driver.cursorOffset);

    return count;

}

static unsigned int cwd_read(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    count = string_length(driver.cwdname);

    memory_copy(buffer, driver.cwdname, count);

    return count;

}

static unsigned int cwd_write(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    memory_clear(driver.cwdname, 128);
    memory_copy(driver.cwdname, buffer, count);

    return count;

}

void init()
{

    tty_driver_init(&driver, "/ramdisk/home/");
    modules_register_driver(&driver.base);

    nodefs_register_node(&in, "tty/stdin", &driver.base.base, in_read, in_write);
    nodefs_register_node(&out, "tty/stdout", &driver.base.base, 0, out_write);
    nodefs_register_node(&cwd, "tty/cwd", &driver.base.base, cwd_read, cwd_write);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

