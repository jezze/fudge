#include <memory.h>
#include <vfs.h>
#include <base/base.h>
#include <video/video.h>
#include <arch/x86/vga/vga.h>

static struct vga_driver driver;

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

    vga_init_driver(&driver);
    video_register_interface(&driver.interface);
    base_register_driver(&driver.base);

}

void destroy()
{

    video_unregister_interface(&driver.interface);
    base_unregister_driver(&driver.base);

}

