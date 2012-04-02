#include <fudge.h>

#define XRES 800
#define YRES 600
#define BPP 32

static void draw_pixel(unsigned int x, unsigned int y, unsigned int color)
{

    unsigned int offset = (y * XRES + x);

    unsigned int fd = file_open("/module/bga/lfb");
    file_write(fd, offset, 4, &color);
    file_close(fd);

}

void set_xres(unsigned int xres)
{

    unsigned int fd = file_open("/module/bga/xres");
    file_write(fd, 0, 4, &xres);
    file_close(fd);

}

void set_yres(unsigned int yres)
{

    unsigned int fd = file_open("/module/bga/yres");
    file_write(fd, 0, 4, &yres);
    file_close(fd);

}

void set_bpp(unsigned int bpp)
{

    unsigned int fd = file_open("/module/bga/bpp");
    file_write(fd, 0, 4, &bpp);
    file_close(fd);

}

void enable()
{

    unsigned int enable = 1;

    unsigned int fd = file_open("/module/bga/enable");
    file_write(fd, 0, 4, &enable);
    file_close(fd);

}

void main(int argc, char *argv[])
{

    unsigned int xres = XRES;
    unsigned int yres = YRES;
    unsigned int bpp = BPP;

    set_xres(xres);
    set_yres(yres);
    set_bpp(bpp);
    enable();

    unsigned int i;
    unsigned int j;

    for (i = 10; i < 20; i++)
        for (j = 10; j < 20; j++)
            draw_pixel(i, j, 0x00FF0000);

}

