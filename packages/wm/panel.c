#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "draw.h"
#include "text.h"
#include "panel.h"

void panel_draw(struct panel *panel)
{

    struct box frameborder;
    struct box frame;
    struct box frametitle;

    box_setsize(&frameborder, panel->size.x, panel->size.y, panel->size.w, panel->size.h);
    box_setsize(&frame, frameborder.x + panel->border, frameborder.y + panel->border, frameborder.w - (2 * panel->border), frameborder.h - (2 * panel->border));
    box_setsize(&frametitle, frame.x + panel->border, frame.y + panel->border, frame.w - (2 * panel->border), frame.h - (2 * panel->border));
    backbuffer_fillbox(&frameborder, WM_COLOR_DARK);

    if (panel->active)
    {

        backbuffer_fillbox(&frame, WM_COLOR_ACTIVEFRAME);
        backbuffer_fillbox(&frametitle, WM_COLOR_ACTIVEFRAMETITLE);

    }

    else
    {

        backbuffer_fillbox(&frame, WM_COLOR_PASSIVEFRAME);
        backbuffer_fillbox(&frametitle, WM_COLOR_PASSIVEFRAMETITLE);

    }

}

void panel_init(struct panel *panel, char *text, unsigned int active, unsigned int border)
{

    box_setsize(&panel->size, 0, 0, 0, 0);

    panel->active = active;
    panel->title.content = text;
    panel->border = border;

}

