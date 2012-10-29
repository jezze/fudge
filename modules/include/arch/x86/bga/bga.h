#ifndef MODULES_BGA_H
#define MODULES_BGA_H

#define BGA_COMMAND                     0x01CE
#define BGA_DATA                        0x01CF

#define BGA_COMMAND_ID                  0
#define BGA_COMMAND_XRES                1
#define BGA_COMMAND_YRES                2
#define BGA_COMMAND_BPP                 3
#define BGA_COMMAND_ENABLE              4
#define BGA_COMMAND_BANK                5
#define BGA_COMMAND_VIRT_WIDTH          6
#define BGA_COMMAND_VIRT_HEIGHT         7
#define BGA_COMMAND_XOFF                8
#define BGA_COMMAND_YOFF                9

#define BGA_BPP_4                       0x04
#define BGA_BPP_8                       0x08
#define BGA_BPP_15                      0x0F
#define BGA_BPP_16                      0x10
#define BGA_BPP_24                      0x18
#define BGA_BPP_32                      0x20

#define BGA_DRIVER_TYPE                 0x0593

struct bga_driver
{

    struct base_driver base;
    struct video_interface interface;
    void *bank;
    void *lfb;
    void (*set_mode)(struct bga_driver *self);

};

void bga_init_driver(struct bga_driver *driver);

#endif

