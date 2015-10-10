#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"
#include "panel.h"

void panel_init(struct panel *panel)
{

    renderable_init(&panel->base, RENDERABLE_TYPE_PANEL, sizeof (struct panel_header));

}

