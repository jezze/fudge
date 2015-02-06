#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "draw.h"
#include "text.h"
#include "panel.h"
#include "window.h"
#include "view.h"

void view_addwindow(struct view *v, struct window *w)
{

    list_add(&v->windows, &w->item);

}

void view_init(struct view *view, char *text, unsigned int active)
{

    panel_init(&view->panel, text, active);
    list_init(&view->windows);

    view->active = active;
    view->windowactive = 0;

}

