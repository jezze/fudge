#include <fudge.h>
#include <gfx/gfx.h>
#include "wm.h"

static struct gfx_backend backend;
static struct gfx_surface rootSurface;
static struct gfx_surface helloSurface;

static struct window rootWindow;
static struct window helloWindow;

static void rootwindow_draw(struct window *self)
{

    gfx_set_color(self->surface, SCREEN_BACKGROUND);
    gfx_set_rectangle(self->surface, self->x, self->y, self->width, self->height);
    gfx_fill(self->surface);

}

static void hellowindow_draw(struct window *self)
{

    gfx_set_color(self->surface, WINDOW_BACKGROUND);
    gfx_set_rectangle(self->surface, self->x, self->y, self->width, self->height);
    gfx_fill(self->surface);

}

static void init_window(struct window *window, unsigned int x, unsigned int y, unsigned int width, unsigned int height, struct gfx_surface *surface, void (*draw)(struct window *self))
{

    memory_clear(window, sizeof (struct window));

    window->x = x;
    window->y = y;
    window->width = width;
    window->height = height;
    window->surface = surface;
    window->draw = draw;

}

static void enable(unsigned int xres, unsigned int yres, unsigned int bpp)
{

    unsigned int enable = 1;

    call_open(3, FUDGE_ROOT, 19, "system/video/1/xres");
    call_write(3, 0, 4, &xres);
    call_open(3, FUDGE_ROOT, 19, "system/video/1/yres");
    call_write(3, 0, 4, &yres);
    call_open(3, FUDGE_ROOT, 18, "system/video/1/bpp");
    call_write(3, 0, 4, &bpp);
    call_open(3, FUDGE_ROOT, 21, "system/video/1/enable");
    call_write(3, 0, 4, &enable);
    call_close(3);

}

void main()
{

    unsigned int id = call_open(4, FUDGE_ROOT, 19, "system/video/1/data");

    if (!id)
        return;

    enable(SCREEN_WIDTH, SCREEN_HEIGHT, 32);
    gfx_init_backend(&backend, id);
    gfx_init_surface(&rootSurface, SCREEN_WIDTH, SCREEN_HEIGHT, GFX_ARGB32, &backend);
    gfx_init_surface(&helloSurface, SCREEN_WIDTH, SCREEN_HEIGHT, GFX_ARGB32, &backend);
    init_window(&rootWindow, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, &rootSurface, rootwindow_draw);
    init_window(&helloWindow, 64, 64, 320, 240, &helloSurface, hellowindow_draw);
    rootWindow.draw(&rootWindow);
    helloWindow.draw(&helloWindow);

}

