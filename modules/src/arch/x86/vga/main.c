#include <vfs.h>
#include <modules/modules.h>
#include <nodefs/nodefs.h>
#include <arch/x86/vga/vga.h>

static struct vga_driver driver;
static struct nodefs_node console;

unsigned int vga_read_framebuffer(unsigned int offset, unsigned int count, void *buffer)
{

    return driver.read_framebuffer(&driver, offset, count, buffer);

}

unsigned int vga_write_framebuffer(unsigned int offset, unsigned int count, void *buffer)
{

    return driver.write_framebuffer(&driver, offset, count, buffer);

}

void vga_set_cursor_color(unsigned char fg, unsigned char bg)
{

    driver.set_cursor_color(&driver, fg, bg);

}

void vga_set_cursor_offset(unsigned short offset)
{

    driver.set_cursor_offset(&driver, offset);

}

void init()
{

    vga_driver_init(&driver);
    modules_register_driver(&driver.base);

    nodefs_register_node(&console, "vga_console", &driver.base.base, 0, 0);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

