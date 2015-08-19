#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"
#include "draw.h"
#include "window.h"

static void render(struct renderable *self, struct ctrl_videosettings *settings, unsigned int line)
{

    struct window *window = self->data;
    unsigned int offset = (line - self->size.y);

    if (offset > self->size.h / 2)
        offset = self->size.h - offset - 1;

    switch (offset)
    {

    case 0:
        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x, self->size.w);

        break;

    case 1:
        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + 0, 1);
        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + self->size.w - 1, 1);
        draw_fill(settings->bpp, window->framecolor, self->size.x + 1, self->size.w - 2);

        break;

    case 2:
        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + 0, 1);
        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + self->size.w - 1, 1);
        draw_fill(settings->bpp, window->framecolor, self->size.x + 1, 1);
        draw_fill(settings->bpp, window->framecolor, self->size.x + self->size.w - 2, 1);
        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + 2, self->size.w - 4);

        break;

    default:
        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + 0, 1);
        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + self->size.w - 1, 1);
        draw_fill(settings->bpp, window->framecolor, self->size.x + 1, 1);
        draw_fill(settings->bpp, window->framecolor, self->size.x + self->size.w - 2, 1);
        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + 2, 1);
        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + self->size.w - 3, 1);

        break;

    }

}

void window_init(struct window *window, unsigned int color)
{

    renderable_init(&window->base, window, render);

}

