#ifndef MODULES_VGA_H
#define MODULES_VGA_H

#define VGA_FB_ADDRESS 0xB8000
#define VGA_FB_SIZE    2000

#define VGA_DEVICE_TYPE 0x0003

struct vga_device
{

    struct modules_device base;
    unsigned char cursorColor;
    unsigned short cursorOffset;
    unsigned int (*read_framebuffer)(struct vga_device *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_framebuffer)(struct vga_device *self, unsigned int offset, unsigned int count, void *buffer);
    void (*set_cursor_color)(struct vga_device *self, unsigned char fg, unsigned char bg);
    void (*set_cursor_offset)(struct vga_device *self, unsigned short offset);

};

extern void vga_device_init(struct vga_device *device);

#endif

