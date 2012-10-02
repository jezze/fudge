#include <fudge.h>
#include "wm.h"

static unsigned int idlfb;

static char buffer[0x1000];
static struct gfx_memory_surface window;
static struct gfx_video_surface screen;

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

    idlfb = call_open(4, 13, "/video/0/data");

    if (!idlfb)
        return;

    gfx_init_video_surface(&screen, idlfb, SCREEN_WIDTH, SCREEN_HEIGHT, ARGB32);
    gfx_init_memory_surface(&window, buffer, 320, 240, ARGB32);

    set_xres(screen.base.width);
    set_yres(screen.base.height);
    set_bpp(32);
    enable();

    screen.base.fill(&screen.base, SCREEN_BACKGROUND);

    call_attach(0x2C, mouse_event);
    call_idle();

}

