#ifndef ARCH_VGA_H
#define ARCH_VGA_H

#define VGA_FB_ADDRESS 0xB8000
#define VGA_FB_SIZE    2000

struct vga_device
{

    struct modules_module base;
    unsigned char cursorColor;
    unsigned short cursorOffset;
    unsigned int (*read_framebuffer)(char *buffer, unsigned int count, unsigned int offset);
    unsigned int (*write_framebuffer)(char *buffer, unsigned int count, unsigned int offset);
    void (*set_cursor_color)(unsigned char fg, unsigned char bg);
    void (*set_cursor_offset)(unsigned short offset);
    struct file_node nodeFramebuffer;
    struct file_node nodeCursorColor;
    struct file_node nodeCursorOffset;

};

extern void vga_init();

#endif

