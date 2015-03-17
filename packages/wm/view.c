#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "draw.h"
#include "text.h"
#include "panel.h"
#include "window.h"
#include "view.h"

void view_init(struct view *view, char *text, unsigned int active)
{

    list_inititem(&view->item, view);
    panel_init(&view->panel, text, active, BORDERSIZE);
    list_init(&view->windows);

    view->active = active;
    view->windowactive = 0;

}

