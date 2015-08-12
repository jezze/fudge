#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "draw.h"
#include "window.h"

void window_draw(struct window *window, struct ctrl_videosettings *settings, unsigned int line)
{

    if (line < window->size.y || line >= window->size.y + window->size.h)
        return;

    if (line == window->size.y + 0 || line >= window->size.y + window->size.h - 1)
    {

        draw_fill(settings->bpp, WM_COLOR_DARK, window->size.x, window->size.w);

    }

    else if (line == window->size.y + 1 || line >= window->size.y + window->size.h - 2)
    {

        draw_fill(settings->bpp, WM_COLOR_DARK, window->size.x + 0, 1);
        draw_fill(settings->bpp, WM_COLOR_DARK, window->size.x + window->size.w - 1, 1);
        draw_fill(settings->bpp, window->active ? WM_COLOR_ACTIVEFRAME : WM_COLOR_PASSIVEFRAME, window->size.x + 1, window->size.w - 2);

    }

    else if (line == window->size.y + 2 || line >= window->size.y + window->size.h - 3)
    {

        draw_fill(settings->bpp, WM_COLOR_DARK, window->size.x + 0, 1);
        draw_fill(settings->bpp, WM_COLOR_DARK, window->size.x + window->size.w - 1, 1);
        draw_fill(settings->bpp, window->active ? WM_COLOR_ACTIVEFRAME : WM_COLOR_PASSIVEFRAME, window->size.x + 1, 1);
        draw_fill(settings->bpp, window->active ? WM_COLOR_ACTIVEFRAME : WM_COLOR_PASSIVEFRAME, window->size.x + window->size.w - 2, 1);
        draw_fill(settings->bpp, WM_COLOR_DARK, window->size.x + 2, window->size.w - 4);

    }

    else
    {

        draw_fill(settings->bpp, WM_COLOR_DARK, window->size.x + 0, 1);
        draw_fill(settings->bpp, WM_COLOR_DARK, window->size.x + window->size.w - 1, 1);
        draw_fill(settings->bpp, window->active ? WM_COLOR_ACTIVEFRAME : WM_COLOR_PASSIVEFRAME, window->size.x + 1, 1);
        draw_fill(settings->bpp, window->active ? WM_COLOR_ACTIVEFRAME : WM_COLOR_PASSIVEFRAME, window->size.x + window->size.w - 2, 1);
        draw_fill(settings->bpp, WM_COLOR_DARK, window->size.x + 2, 1);
        draw_fill(settings->bpp, WM_COLOR_DARK, window->size.x + window->size.w - 3, 1);
        draw_fill(settings->bpp, WM_COLOR_BODY, window->size.x + 3, window->size.w - 6);

    }

}

void window_activate(struct window *window)
{

    window->active = 1;

}

void window_deactivate(struct window *window)
{

    window->active = 0;

}

void window_init(struct window *window)
{

    list_inititem(&window->item, window);
    box_setsize(&window->size, 0, 0, 0, 0);
    box_setsize(&window->screen, 0, 0, 0, 0);

    window->active = 0;
    window->source = 0;

}

