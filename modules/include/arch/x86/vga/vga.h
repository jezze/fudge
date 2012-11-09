#define VGA_FB_ADDRESS                  0xB8000
#define VGA_FB_SIZE                     2000
#define VGA_FB_COLOR_BLACK              0x00
#define VGA_FB_COLOR_BLUE               0x01
#define VGA_FB_COLOR_BLUE2              0x09
#define VGA_FB_COLOR_BROWN              0x06
#define VGA_FB_COLOR_BROWN2             0x0E
#define VGA_FB_COLOR_CYAN               0x03
#define VGA_FB_COLOR_CYAN2              0x0B
#define VGA_FB_COLOR_GRAY               0x08
#define VGA_FB_COLOR_GRAY2              0x07
#define VGA_FB_COLOR_GREEN              0x02
#define VGA_FB_COLOR_GREEN2             0x0A
#define VGA_FB_COLOR_MAGENTA            0x05
#define VGA_FB_COLOR_MAGENTA2           0x0D
#define VGA_FB_COLOR_RED                0x04
#define VGA_FB_COLOR_RED2               0x0C
#define VGA_FB_COLOR_WHITE              0x0F

#define VGA_DRIVER_TYPE                 0x0003

struct vga_cursor
{

    unsigned char color;
    unsigned short offset;

};

struct vga_driver
{

    struct base_driver base;
    struct video_interface interface;
    struct vga_cursor cursor;

};

void vga_init_driver(struct vga_driver *driver);
