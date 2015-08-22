#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"
#include "draw.h"
#include "panel.h"

void panel_render(struct renderable *self, struct ctrl_videosettings *settings, unsigned int line)
{

    struct panel *panel = (struct panel *)self;
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
        draw_fill(settings->bpp, panel->framecolor, self->size.x + 1, self->size.w - 2);

        break;

    default:
        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + 0, 1);
        draw_fill(settings->bpp, WM_COLOR_DARK, self->size.x + self->size.w - 1, 1);
        draw_fill(settings->bpp, panel->framecolor, self->size.x + 1, 1);
        draw_fill(settings->bpp, panel->framecolor, self->size.x + self->size.w - 2, 1);
        draw_fill(settings->bpp, panel->backgroundcolor, self->size.x + 2, self->size.w - 4);

        break;

    }

}

void panel_init(struct panel *panel)
{

    renderable_init(&panel->base, RENDERABLE_TYPE_PANEL, sizeof (struct panel) - sizeof (struct renderable));

    panel->backgroundcolor = WM_COLOR_PASSIVEBACK;
    panel->framecolor = WM_COLOR_PASSIVEFRAME;

}

