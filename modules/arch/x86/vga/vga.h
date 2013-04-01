#define VGA_ADDRESS                     0x000A0000

#define VGA_REG_CR_MONO_INDEX           0x03B4
#define VGA_REG_CR_MONO_DATA            0x03B5
#define VGA_REG_FC_MONO_CTRL            0x03BA
#define VGA_REG_AR_INDEX                0x03C0
#define VGA_REG_AR_DATA                 0x03C1
#define VGA_REG_MISC_CTRL               0x03C2
#define VGA_REG_SR_INDEX                0x03C4
#define VGA_REG_SR_DATA                 0x03C5
#define VGA_REG_DAC_PIXMASK             0x03C6
#define VGA_REG_DAC_STATUS              0x03C7
#define VGA_REG_DAC_CTRL                0x03C8
#define VGA_REG_DAC_DATA                0x03C9
#define VGA_REG_FC_STATUS               0x03CA
#define VGA_REG_MISC_DATA               0x03CC
#define VGA_REG_GR_INDEX                0x03CE
#define VGA_REG_GR_DATA                 0x03CF
#define VGA_REG_CR_COLOR_INDEX          0x03D4
#define VGA_REG_CR_COLOR_DATA           0x03D5
#define VGA_REG_FC_COLOR_CTRL           0x03DA

#define VGA_MISC_MONO                   (0 << 0)
#define VGA_MISC_COLOR                  (1 << 0)
#define VGA_MISC_DISABLE                (0 << 1)
#define VGA_MISC_ENABLE                 (1 << 1)
#define VGA_MISC_VCLK0                  (0 << 2)
#define VGA_MISC_VCLK1                  (1 << 2)
#define VGA_MISC_VCLK2                  (2 << 2)
#define VGA_MISC_VCLK3                  (3 << 2)
#define VGA_MISC_PAGESELECT             (1 << 5)
#define VGA_MISC_SYNC400                (1 << 6)
#define VGA_MISC_SYNC350                (2 << 6)
#define VGA_MISC_SYNC480                (3 << 6)

#define VGA_FB_ADDRESS                  0x000B8000
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

struct vga_cursor
{

    unsigned char color;
    unsigned short offset;

};

struct vga_driver
{

    struct base_driver base;
    struct video_interface video;
    struct terminal_interface terminal;
    struct vga_cursor cursor;

};

void vga_init_driver(struct vga_driver *driver);
