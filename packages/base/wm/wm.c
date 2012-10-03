#include <fudge.h>
#include "wm.h"

static struct gfx_backend backend;
static struct gfx_surface rootSurface;
static struct gfx_surface helloSurface;

static struct window rootWindow;
static struct window helloWindow;

static void rootwindow_draw(struct window *self)
{

    gfx_set_color(self->surface, SCREEN_BACKGROUND);
    gfx_fill(self->surface);

}

static void hellowindow_draw(struct window *self)
{

    gfx_set_rectangle(self->surface, self->x, self->y, 32, 32);
    gfx_set_color(self->surface, WINDOW_BACKGROUND);
    gfx_fill(self->surface);

}

static void init_window(struct window *window, unsigned int x, unsigned int y, struct gfx_surface *surface, void (*draw)(struct window *self))
{

    memory_clear(window, sizeof (struct window));

    window->x = x;
    window->y = y;
    window->surface = surface;
    window->draw = draw;

}


static void set_xres(unsigned int xres)
{

    call_open(3, 13, "/video/0/xres");
    call_write(3, 0, 4, &xres);
    call_close(3);

}

static void set_yres(unsigned int yres)
{

    call_open(3, 13, "/video/0/yres");
    call_write(3, 0, 4, &yres);
    call_close(3);

}

static void set_bpp(unsigned int bpp)
{

    call_open(3, 12, "/video/0/bpp");
    call_write(3, 0, 4, &bpp);
    call_close(3);

}

static void enable()
{

    unsigned int enable = 1;

    call_open(3, 15, "/video/0/enable");
    call_write(3, 0, 4, &enable);
    call_close(3);

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

    gfx_init_backend(&backend, id);
    gfx_init_surface(&rootSurface, SCREEN_WIDTH, SCREEN_HEIGHT, GFX_ARGB32, &backend);
    gfx_init_surface(&helloSurface, SCREEN_WIDTH, SCREEN_HEIGHT, GFX_ARGB32, &backend);

    set_xres(rootSurface.width);
    set_yres(rootSurface.height);
    set_bpp(32);
    enable();

    init_window(&rootWindow, 0, 0, &rootSurface, rootwindow_draw);
    rootWindow.draw(&rootWindow);

    init_window(&helloWindow, 64, 64, &helloSurface, hellowindow_draw);
    helloWindow.draw(&helloWindow);

    call_attach(0x2C, mouse_event);
    call_idle();

}

