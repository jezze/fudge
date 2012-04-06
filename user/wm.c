#include <fudge.h>
#include <ppm.h>

#define XRES 800
#define YRES 600
#define BPP 32

static unsigned int fdbuf;
static unsigned int fdx;
static unsigned int fdy;

static int mx;
static int my;

static void draw_pixel(unsigned int x, unsigned int y, unsigned int color)
{

    unsigned int offset = (y * XRES + x) * 4;

    file_write(fdbuf, offset, 4, &color);

}

static void draw_buffer(unsigned int x, unsigned int y, unsigned int count, void *buffer)
{

    unsigned int offset = (y * XRES + x) * 4;

    file_write(fdbuf, offset, count, buffer);

}

static void draw_fill(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color)
{

    unsigned char buffer[0xC80];

    unsigned int width = x2 - x1;
    unsigned int height = y2 - y1;

    unsigned int i;

    for (i = 0; i < width; i++)
        memory_copy(buffer + i * 4, &color, 4);

    for (i = y1; i < y1 + height; i++)
        draw_buffer(x1, i, width * 4, &buffer);

}

static void draw_ppm(char *name, unsigned int x, unsigned int y)
{

    unsigned char buffer[0xC80];

    unsigned int fd = file_open(name);
    file_read(fd, 0, 0xC80, buffer);

    struct ppm_header header;

    ppm_parse(&header, buffer);

    header.width = 20;
    header.height = 20;
    unsigned int offset = 0x34;

    unsigned int count = file_read(fd, offset, 0xC80, buffer);

    int cx = x;
    int cy = y;

    unsigned int i;

    for (i = 1; i < count; i += 3)
    {

        draw_pixel(cx, cy, *(unsigned int *)(buffer + i));

        cx++;

        if (cx == x + header.width)
        {

            cx = x;
            cy++;

        }

    }

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

    char dx;
    char dy;

    file_read(fdx, 0, 1, &dx);
    file_read(fdy, 0, 1, &dy);

    mx += (dx * 0.2);
    my -= (dy * 0.2);

    draw_ppm("/ramdisk/home/fu-raw.ppm", mx, my);

    call_wait();

}

void main(int argc, char *argv[])
{

    fdbuf = file_open("/module/bga/lfb");

    if (!fdbuf)
        return;

    fdx = file_open("/module/ps2/mx");
    fdy = file_open("/module/ps2/my");

    unsigned int xres = XRES;
    unsigned int yres = YRES;
    unsigned int bpp = BPP;

    set_xres(xres);
    set_yres(yres);
    set_bpp(bpp);
    enable();

    draw_fill(0, 0, xres, yres, 0x00FFFFFF);

    mx = 400;
    my = 300;

    call_attach(0x0C, mouse);
    call_wait();

}

