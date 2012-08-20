#include <fudge.h>
#include "wm.h"

static unsigned int idlfb;
static unsigned int idmcycle;
static unsigned int idmstatus;
static unsigned int idmx;
static unsigned int idmy;

static char buffer[0x2000];
static char buffer2[0x2000];

static float mx;
static float my;

static unsigned int get_offset(unsigned int x, unsigned int y)
{

    return (y * SCREEN_WIDTH + x) * SCREEN_BSIZE;

}

static void read_buffer(unsigned int x, unsigned int y, unsigned int count, void *buffer)
{

    unsigned int offset = get_offset(x, y);

    call_read(idlfb, offset, count, buffer);

}

static void read_buffer_rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, void *buffer)
{

    unsigned int i;

    for (i = 0; i < h; i++)
        read_buffer(x, y + i, w * SCREEN_BSIZE, (char *)buffer + w * i * SCREEN_BSIZE);

}

static void write_buffer(unsigned int x, unsigned int y, unsigned int count, void *buffer)
{

    unsigned int offset = get_offset(x, y);

    call_write(idlfb, offset, count, buffer);

}

static void write_buffer_rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, void *buffer)
{

    unsigned int i;

    for (i = 0; i < h; i++)
        write_buffer(x, y + i, w * SCREEN_BSIZE, (char *)buffer + w * i * SCREEN_BSIZE);

}

static void fill_buffer(unsigned int count, void *buffer, unsigned int color)
{

    unsigned int i;

    for (i = 0; i < count; i++)
        memory_copy((char *)buffer + i * SCREEN_BSIZE, &color, SCREEN_BSIZE);

}

static void write_fill(unsigned int x, unsigned int y, unsigned int w, unsigned int h, void *buffer, unsigned int color)
{

    unsigned int i;

    fill_buffer(w, buffer, color);

    for (i = 0; i < h; i++)
        write_buffer(x, y + i, w * SCREEN_BSIZE, buffer);

}

void set_xres(unsigned int xres)
{

    call_open(3, 16, "/module/bga_xres");
    call_write(3, 0, 4, &xres);
    call_close(3);

}

void set_yres(unsigned int yres)
{

    call_open(3, 16, "/module/bga_yres");
    call_write(3, 0, 4, &yres);
    call_close(3);

}

void set_bpp(unsigned int bpp)
{

    call_open(3, 15, "/module/bga_bpp");
    call_write(3, 0, 4, &bpp);
    call_close(3);

}

void enable()
{

    unsigned int enable = 1;

    call_open(3, 18, "/module/bga_enable");
    call_write(3, 0, 4, &enable);
    call_close(3);

}

void set_mouse_coords(float x, float y)
{

    mx = x;
    my = y;

    if (mx < 0)
        mx = 0;

    if (mx > SCREEN_WIDTH - 10)
        mx = SCREEN_WIDTH - 10;

    if (my < 0)
        my = 0;

    if (my > SCREEN_HEIGHT - 10)
        my = SCREEN_HEIGHT - 10;

}

void mouse_event()
{

    unsigned char cycle;
    unsigned char status;
    unsigned char dx;
    unsigned char dy;
    int relx;
    int rely;

    call_read(idmcycle, 0, 1, &cycle);

    if (cycle)
        call_idle();

    call_read(idmstatus, 0, 1, &status);
    call_read(idmx, 0, 1, &dx);
    call_read(idmy, 0, 1, &dy);

    write_buffer_rect((unsigned int)mx, (unsigned int)my, 10, 10, buffer);

    relx = dx - ((status << 4) & 0x100);
    rely = dy - ((status << 3) & 0x100);

    set_mouse_coords(mx + relx, my - rely);

    read_buffer_rect((unsigned int)mx, (unsigned int)my, 10, 10, buffer);

    write_fill((unsigned int)mx, (unsigned int)my, 10, 10, buffer2, 0xFFFFFF);

    call_idle();

}

void main()
{

    idlfb = call_open(4, 15, "/module/bga_lfb");

    if (!idlfb)
        return;

    idmcycle = call_open(5, 18, "/module/ps2_mcycle");
    idmstatus = call_open(6, 19, "/module/ps2_mstatus");
    idmx = call_open(7, 14, "/module/ps2_mx");
    idmy = call_open(8, 14, "/module/ps2_my");

    set_xres(SCREEN_WIDTH);
    set_yres(SCREEN_HEIGHT);
    set_bpp(SCREEN_BPP);
    enable();

    set_mouse_coords(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

    write_fill(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, buffer2, SCREEN_BACKGROUND);
    write_fill(100, 200, 300, 200, buffer2, WINDOW_BACKGROUND);

    read_buffer_rect((unsigned int)mx, (unsigned int)my, 10, 10, buffer);

    call_attach(0x2C, mouse_event);
    call_idle();

}

