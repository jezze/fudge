#ifndef MODULES_BGA_H
#define MODULES_BGA_H

#define BGA_INDEX 0x01CE
#define BGA_DATA  0x01CF

#define BGA_INDEX_ID          0
#define BGA_INDEX_XRES        1
#define BGA_INDEX_YRES        2
#define BGA_INDEX_BPP         3
#define BGA_INDEX_ENABLE      4
#define BGA_INDEX_BANK        5
#define BGA_INDEX_VIRT_WIDTH  6
#define BGA_INDEX_VIRT_HEIGHT 7
#define BGA_INDEX_XOFF        8
#define BGA_INDEX_YOFF        9

#define BGA_BPP_4  0x04
#define BGA_BPP_8  0x08
#define BGA_BPP_15 0x0F
#define BGA_BPP_16 0x10
#define BGA_BPP_24 0x18
#define BGA_BPP_32 0x20

#define BGA_DRIVER_TYPE 593

struct bga_driver
{

    struct modules_driver base;
    unsigned int *bank;
    unsigned int xres;
    unsigned int yres;
    unsigned int bpp;
    void (*set_mode)(struct bga_driver *self, unsigned int xres, unsigned int yres, unsigned int bpp);
    void (*set_bank)(struct bga_driver *self, unsigned int index);

};

extern void bga_driver_init(struct bga_driver *driver);

#endif

