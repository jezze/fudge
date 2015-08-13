#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"
#include "draw.h"
#include "panel.h"

static void render(struct renderable *self, struct ctrl_videosettings *settings, unsigned int line)
{

    struct panel *panel = self->data;

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
        draw_fill(settings->bpp, panel->active ? WM_COLOR_ACTIVEFRAME : WM_COLOR_PASSIVEFRAME, self->size.x + 1, self->size.w - 2);

    }

    else
    {

        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + 0, 1);
        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + self->size.w - 1, 1);
        draw_fill(settings->bpp, panel->active ? WM_COLOR_ACTIVEFRAME : WM_COLOR_PASSIVEFRAME, self->size.x + 1, 1);
        draw_fill(settings->bpp, panel->active ? WM_COLOR_ACTIVEFRAME : WM_COLOR_PASSIVEFRAME, self->size.x + self->size.w - 2, 1);
        draw_fill(settings->bpp, panel->active ? WM_COLOR_ACTIVEBACK : WM_COLOR_PASSIVEBACK, self->size.x + 2, self->size.w - 4);

    }

}

void panel_activate(struct panel *panel)
{

    panel->active = 1;

}

void panel_deactivate(struct panel *panel)
{

    panel->active = 0;

}

void panel_init(struct panel *panel)
{

    renderable_init(&panel->base, panel, render);

}

