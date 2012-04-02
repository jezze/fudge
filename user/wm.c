#include <fudge.h>

#define XRES 800
#define YRES 600
#define BPP 32

static unsigned int mx;
static unsigned int my;

static void draw_box(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color)
{

    unsigned char buffer[0xC80];

    unsigned int width = x2 - x1;
    unsigned int height = y2 - y1;

    unsigned int i;

    for (i = 0; i < width; i++)
        memory_copy(buffer + i * 4, &color, 4);

    unsigned int fd = file_open("/module/bga/lfb");

    for (i = y1; i < y1 + height; i++)
    {

        unsigned int offset = (i * XRES + x1) * 4;

        file_write(fd, offset, width * 4, &buffer);

    }

    file_close(fd);


}

static void draw_pixel(unsigned int x, unsigned int y, unsigned int color)
{

    unsigned int offset = (y * XRES + x) * 4;

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

void mouse()
{

    draw_pixel(mx, mx, 0x00FF0000);

    mx++;
    my++;

    call_wait();

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

    draw_box(0, 0, xres, yres, 0x00223344);

    mx = 240;
    my = 240;

    call_attach(0x0C, mouse);
    call_wait();

}

