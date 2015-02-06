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
    box_setsize(&frame, frameborder.x + 1, frameborder.y + 1, frameborder.w - 2, frameborder.h - 2);
    box_setsize(&frametitle, frame.x + 1, frame.y + 1, frame.w - 2, frame.h - 2);
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

void panel_init(struct panel *panel, char *text, unsigned int active)
{

    box_setsize(&panel->size, 0, 0, 0, 0);

    panel->active = active;
    panel->title.content = text;

}

