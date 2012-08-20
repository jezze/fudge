#include <fudge.h>
#include "wm.h"

static unsigned int idlfb;
static unsigned int idmcycle;
static unsigned int idmstatus;
static unsigned int idmx;
static unsigned int idmy;

static char buffer[0x2000];

static float mx;
static float my;

static void draw_pixel(unsigned int x, unsigned int y, unsigned int color)
{

    unsigned int offset = (y * SCREEN_WIDTH + x) * 4;

    call_write(idlfb, offset, 4, &color);

}

static void draw_buffer(unsigned int x, unsigned int y, unsigned int count, void *buffer)
{

    unsigned int offset = (y * SCREEN_WIDTH + x) * 4;

    call_write(idlfb, offset, count, buffer);

}

static void draw_stroke(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color)
{

    unsigned int width = x2 - x1 + 1;
    unsigned int i;

    for (i = 0; i <= width; i++)
        memory_copy(buffer + i * 4, &color, 4);

    draw_buffer(x1, y1, width * 4, &buffer);

    for (i = y1; i <= y2; i++)
    {

        draw_pixel(x1, i, color);
        draw_pixel(x2, i, color);

    }

    draw_buffer(x1, y2, width * 4, &buffer);

}

static void draw_fill(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color)
{

    unsigned int width = x2 - x1 + 1;
    unsigned int i;

    for (i = 0; i <= width; i++)
        memory_copy(buffer + i * 4, &color, 4);

    for (i = y1; i <= y2; i++)
        draw_buffer(x1, i, width * 4, &buffer);

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

void set_mouse_coords(unsigned int x, unsigned int y)
{

    mx = x;
    my = y;

    if (mx > SCREEN_WIDTH)
        mx = SCREEN_WIDTH;

    if (my > SCREEN_HEIGHT)
        my = SCREEN_HEIGHT;

}

void mouse_event()
{

    char cycle;
    char status;
    char dx;
    char dy;

    call_read(idmcycle, 0, 1, &cycle);

    if (cycle)
        call_idle();

    call_read(idmstatus, 0, 1, &status);
    call_read(idmx, 0, 1, &dx);
    call_read(idmy, 0, 1, &dy);

    dx = dx / 2;
    dy = dy / 2;

    set_mouse_coords(mx + dx, my - dy);

    draw_fill(mx, my, mx + 10, my + 10, 0xFFFFFF);

    call_idle();

}

void draw_frame(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    draw_fill(x, y, w, h, FRAME_BACKGROUND);
    draw_stroke(x + 0, y + 0, w - 0, h - 0, FRAME_BORDER_DARK);
    draw_stroke(x + 1, y + 1, w - 1, h - 1, FRAME_BORDER_LIGHT);
    draw_stroke(x + 5, y + 35, w - 5, h - 5, FRAME_BORDER_DARK);
    draw_stroke(x + 4, y + 34, w - 4, h - 4, FRAME_BORDER_LIGHT);

}

void draw_panel(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    draw_fill(x, y, w, h, PANEL_BACKGROUND);
    draw_stroke(x, y, w, h, PANEL_BORDER_DARK);
    draw_stroke(x + 1, y + 1, w - 1, h - 1, PANEL_BORDER_LIGHT);

}

void draw_window(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    draw_frame(x, y, w, h);
    draw_fill(x + 6, y + 36, w - 6, h - 6, WINDOW_BACKGROUND);
    draw_panel(x + 24, y + 54, w - 24, y + 112);
    draw_panel(x + 24, y + 112, w - 24, h - 24);

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

    draw_fill(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BACKGROUND);
    draw_window(200, 100, 600, 400);

    call_attach(0x2C, mouse_event);
    call_idle();

}

