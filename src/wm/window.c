#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"
#include "draw.h"
#include "window.h"

static void render(struct renderable *self, struct ctrl_videosettings *settings, unsigned int line)
{

    struct window *window = self->data;

    if (line < self->size.y || line >= self->size.y + self->size.h)
        return;

    if (line == self->size.y + 0 || line >= self->size.y + self->size.h - 1)
    {

        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x, self->size.w);

    }

    else if (line == self->size.y + 1 || line >= self->size.y + self->size.h - 2)
    {

        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + 0, 1);
        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + self->size.w - 1, 1);
        draw_fill(settings->bpp, window->framecolor, self->size.x + 1, self->size.w - 2);

    }

    else if (line == self->size.y + 2 || line >= self->size.y + self->size.h - 3)
    {

        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + 0, 1);
        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + self->size.w - 1, 1);
        draw_fill(settings->bpp, window->framecolor, self->size.x + 1, 1);
        draw_fill(settings->bpp, window->framecolor, self->size.x + self->size.w - 2, 1);
        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + 2, self->size.w - 4);

    }

    else
    {

        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + 0, 1);
        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + self->size.w - 1, 1);
        draw_fill(settings->bpp, window->framecolor, self->size.x + 1, 1);
        draw_fill(settings->bpp, window->framecolor, self->size.x + self->size.w - 2, 1);
        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + 2, 1);
        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + self->size.w - 3, 1);

    }

}

void window_init(struct window *window, unsigned int color)
{

    renderable_init(&window->base, window, render);

}

