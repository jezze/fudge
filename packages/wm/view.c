#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "draw.h"
#include "panel.h"
#include "window.h"
#include "view.h"

static void drawwindows(struct list *windows, unsigned int line)
{

    struct list_item *current;

    for (current = windows->head; current; current = current->next)
    {

        struct window *window = current->data;

        window_draw(window, line);

    }

}

void view_draw(struct view *view, unsigned int line)
{

    panel_draw(&view->panel, line);

    if (!view->active)
        return;

    drawwindows(&view->windows, line);

}

void view_init(struct view *view)
{

    list_inititem(&view->item, view);
    list_init(&view->windows);
    panel_init(&view->panel);

    view->active = 0;
    view->windowactive = 0;

}

