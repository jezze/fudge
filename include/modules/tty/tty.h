#ifndef MODULES_TTY_H
#define MODULES_TTY_H

#define TTY_CHARACTER_WIDTH  80
#define TTY_CHARACTER_HEIGHT 25
#define TTY_CHARACTER_SIZE   2000
#define TTY_COLOR_BLACK      0x00
#define TTY_COLOR_BLUE       0x01
#define TTY_COLOR_BLUE2      0x09
#define TTY_COLOR_BROWN      0x06
#define TTY_COLOR_BROWN2     0x0e
#define TTY_COLOR_CYAN       0x03
#define TTY_COLOR_CYAN2      0x0b
#define TTY_COLOR_GRAY       0x08
#define TTY_COLOR_GRAY2      0x07
#define TTY_COLOR_GREEN      0x02
#define TTY_COLOR_GREEN2     0x0a
#define TTY_COLOR_MAGENTA    0x05
#define TTY_COLOR_MAGENTA2   0x0d
#define TTY_COLOR_RED        0x04
#define TTY_COLOR_RED2       0x0c
#define TTY_COLOR_WHITE      0x0f
#define TTY_CWD_SIZE         256

#define TTY_DEVICE_TYPE 1000

struct tty_device
{

    struct modules_device base;
    char cwdname[32];
    unsigned short cursorOffset;
    struct serial_device *serialDevice;
    struct kbd_device *kbdDevice;
    struct vga_device *vgaDevice;
    struct vfs_view nview;
    struct vfs_node nin;
    struct vfs_node nout;
    struct vfs_node nerror;
    struct vfs_node ncwd;
    struct vfs_node npwd;

};

extern void tty_device_init(struct tty_device *device, char *cwdname);

#endif
