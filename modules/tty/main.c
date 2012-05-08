#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/modules.h>
#include <modules/nodefs/nodefs.h>
#include <modules/ps2/ps2.h>
#include <modules/vga/vga.h>
#include <modules/tty/tty.h>

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

        if (!driver.kbdDriver->buffer.getc(&driver.kbdDriver->buffer, stream + i))
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

        if (!driver.kbdDriver->buffer.putc(&driver.kbdDriver->buffer, stream + i))
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

    driver.vgaDriver->set_cursor_offset(driver.vgaDriver, driver.cursorOffset);

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

    struct ps2_kbd_driver *kbdDriver = (struct ps2_kbd_driver *)modules_get_driver(PS2_KBD_DRIVER_TYPE);

    if (!kbdDriver)
        return;

    struct vga_driver *vgaDriver = (struct vga_driver *)modules_get_driver(VGA_DRIVER_TYPE);

    if (!vgaDriver)
        return;

    tty_driver_init(&driver, kbdDriver, vgaDriver, "/ramdisk/home/");
    modules_register_driver(&driver.base);

    struct nodefs_driver *nodefsDriver = (struct nodefs_driver *)modules_get_driver(NODEFS_DRIVER_TYPE);

    if (!nodefsDriver)
        return;

    nodefsDriver->register_node(nodefsDriver, &in, "tty/stdin", &driver.base.base, in_read, in_write);
    nodefsDriver->register_node(nodefsDriver, &out, "tty/stdout", &driver.base.base, 0, out_write);
    nodefsDriver->register_node(nodefsDriver, &cwd, "tty/cwd", &driver.base.base, cwd_read, cwd_write);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

