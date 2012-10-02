#include <fudge.h>
#include "wm.h"

static struct gfx_backend backend;
static struct gfx_surface rootSurface;
static struct gfx_window rootWindow;
static struct gfx_surface helloSurface;
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

void rootwindow_draw(struct gfx_window *self)
{

    gfx_set_color(self->surface, SCREEN_BACKGROUND);
    gfx_fill(self->surface);

}

void hellowindow_draw(struct gfx_window *self)
{

    gfx_set_color(self->surface, SCREEN_BACKGROUND);
    gfx_fill(self->surface);

}

void main()
{

    unsigned int id = call_open(4, 13, "/video/0/data");

    if (!id)
        return;

    gfx_init_backend(&backend, id);
    gfx_init_surface(&rootSurface, SCREEN_WIDTH, SCREEN_HEIGHT, ARGB32, &backend);
    gfx_init_window(&rootWindow, 0, 0, &rootSurface, rootwindow_draw);

    set_xres(rootSurface.width);
    set_yres(rootSurface.height);
    set_bpp(32);
    enable();

    rootWindow.draw(&rootWindow);

    gfx_init_surface(&helloSurface, 320, 240, ARGB32, &backend);
    gfx_init_window(&helloWindow, 64, 64, &helloSurface, hellowindow_draw);

    call_attach(0x2C, mouse_event);
    call_idle();

}

