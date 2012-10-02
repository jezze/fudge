#include <fudge.h>
#include "wm.h"

static struct gfx_video_surface rootSurface;
static struct gfx_window rootWindow;
static struct gfx_video_surface helloSurface;
static struct gfx_window helloWindow;

void set_xres(unsigned int xres)
{

    call_open(3, 13, "/video/0/xres");
    call_write(3, 0, 4, &xres);
    call_close(3);

}

void set_yres(unsigned int yres)
{

    call_open(3, 13, "/video/0/yres");
    call_write(3, 0, 4, &yres);
    call_close(3);

}

void set_bpp(unsigned int bpp)
{

    call_open(3, 12, "/video/0/bpp");
    call_write(3, 0, 4, &bpp);
    call_close(3);

}

void enable()
{

    unsigned int enable = 1;

    call_open(3, 15, "/video/0/enable");
    call_write(3, 0, 4, &enable);
    call_close(3);

}

void set_mouse_coords(unsigned int x, unsigned int y)
{

}

void mouse_event()
{

    call_idle();

}

void main()
{

    unsigned int id = call_open(4, 13, "/video/0/data");

    if (!id)
        return;

    gfx_init_video_surface(&rootSurface, id, SCREEN_WIDTH, SCREEN_HEIGHT, ARGB32);
    gfx_init_window(&rootWindow, 0, 0, &rootSurface.base);

    set_xres(rootSurface.base.width);
    set_yres(rootSurface.base.height);
    set_bpp(32);
    enable();

    rootSurface.base.fill(&rootSurface.base, SCREEN_BACKGROUND);

    gfx_init_video_surface(&helloSurface, id, 320, 240, ARGB32);
    gfx_init_window(&helloWindow, 64, 64, &helloSurface.base);

    call_attach(0x2C, mouse_event);
    call_idle();

}

