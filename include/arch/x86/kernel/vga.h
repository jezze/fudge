#ifndef ARCH_VGA_H
#define ARCH_VGA_H

#define VGA_FB_ADDRESS          0xB8000
#define VGA_FB_CHARACTER_WIDTH  80
#define VGA_FB_CHARACTER_HEIGHT 25
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

extern void vga_fb_set_cursor_offset(unsigned short offset);
extern void vga_init();

#endif

