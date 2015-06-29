#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "draw.h"
#include "panel.h"
#include "window.h"
#include "view.h"

static void drawwindows(struct list *windows, unsigned int bpp, unsigned int line)
{

    struct list_item *current;

    for (current = windows->head; current; current = current->next)
    {

        struct window *window = current->data;

        window_draw(window, bpp, line);

    }

}

void view_draw(struct view *view, unsigned int bpp, unsigned int line)
{

    panel_draw(&view->panel, bpp, line);

    if (!view->active)
        return;

    drawwindows(&view->windows, bpp, line);

}

void view_activate(struct view *view)
{

    panel_activate(&view->panel);

    view->active = 1;

}

void view_deactivate(struct view *view)
{

    panel_deactivate(&view->panel);

    view->active = 0;

}

void view_init(struct view *view, unsigned int center)
{

    list_inititem(&view->item, view);
    list_init(&view->windows);
    panel_init(&view->panel);

    view->center = center;
    view->active = 0;
    view->windowfocus = 0;

}

