#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "draw.h"
#include "text.h"
#include "panel.h"

void panel_draw(struct panel *panel, unsigned int line)
{

    if (panel->active)
        backbuffer_fillbox(&panel->size, WM_COLOR_ACTIVEFRAME, line);
    else
        backbuffer_fillbox(&panel->size, WM_COLOR_PASSIVEFRAME, line);

}

void panel_init(struct panel *panel, char *text, unsigned int active)
{

    box_setsize(&panel->size, 0, 0, 0, 0);

    panel->active = active;
    panel->title.content = text;

}

