#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "draw.h"
#include "panel.h"
#include "window.h"
#include "view.h"

static void drawwindows(struct list *windows, struct ctrl_videosettings *settings, unsigned int line)
{

    struct list_item *current;

    for (current = windows->head; current; current = current->next)
    {

        struct window *window = current->data;

        window_draw(window, settings, line);

    }

}

void view_draw(struct view *view, struct ctrl_videosettings *settings, unsigned int line)
{

    panel_draw(&view->panel, settings, line);

    if (!view->active)
        return;

    drawwindows(&view->windows, settings, line);

}

void view_init(struct view *view, unsigned int center)
{

    list_inititem(&view->item, view);
    list_init(&view->windows);
    panel_init(&view->panel);

    view->center = center;
    view->active = 0;
    view->windowactive = 0;

}

