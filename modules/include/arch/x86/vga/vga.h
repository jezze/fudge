#ifndef MODULES_VGA_H
#define MODULES_VGA_H

#define VGA_FB_ADDRESS          0xB8000
#define VGA_FB_SIZE             2000
#define VGA_FB_COLOR_BLACK      0x00
#define VGA_FB_COLOR_BLUE       0x01
#define VGA_FB_COLOR_BLUE2      0x09
#define VGA_FB_COLOR_BROWN      0x06
#define VGA_FB_COLOR_BROWN2     0x0e
#define VGA_FB_COLOR_CYAN       0x03
#define VGA_FB_COLOR_CYAN2      0x0b
#define VGA_FB_COLOR_GRAY       0x08
#define VGA_FB_COLOR_GRAY2      0x07
#define VGA_FB_COLOR_GREEN      0x02
#define VGA_FB_COLOR_GREEN2     0x0a
#define VGA_FB_COLOR_MAGENTA    0x05
#define VGA_FB_COLOR_MAGENTA2   0x0d
#define VGA_FB_COLOR_RED        0x04
#define VGA_FB_COLOR_RED2       0x0c
#define VGA_FB_COLOR_WHITE      0x0f

#define VGA_DRIVER_TYPE 0x0003

struct vga_driver
{

    struct base_driver base;
    unsigned char cursorColor;
    unsigned short cursorOffset;
    unsigned int (*read_framebuffer)(struct vga_driver *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write_framebuffer)(struct vga_driver *self, unsigned int offset, unsigned int count, void *buffer);
    void (*set_cursor_color)(struct vga_driver *self, unsigned char fg, unsigned char bg);
    void (*set_cursor_offset)(struct vga_driver *self, unsigned short offset);

};

unsigned int vga_read_framebuffer(unsigned int offset, unsigned int count, void *buffer);
unsigned int vga_write_framebuffer(unsigned int offset, unsigned int count, void *buffer);
void vga_set_cursor_color(unsigned char fg, unsigned char bg);
void vga_set_cursor_offset(unsigned short offset);
void vga_driver_init(struct vga_driver *driver);

#endif

