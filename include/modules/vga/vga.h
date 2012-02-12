#ifndef MODULES_VGA_H
#define MODULES_VGA_H

#define VGA_FB_ADDRESS 0xB8000
#define VGA_FB_SIZE    2000

#define VGA_DRIVER_TYPE 0x0003

struct vga_driver
{

    struct modules_driver base;
    unsigned char cursorColor;
    unsigned short cursorOffset;
    unsigned int (*read_framebuffer)(struct vga_driver *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_framebuffer)(struct vga_driver *self, unsigned int offset, unsigned int count, void *buffer);
    void (*set_cursor_color)(struct vga_driver *self, unsigned char fg, unsigned char bg);
    void (*set_cursor_offset)(struct vga_driver *self, unsigned short offset);

};

void vga_driver_init(struct vga_driver *driver);

#endif

