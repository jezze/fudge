#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "draw.h"
#include "panel.h"
#include "window.h"
#include "view.h"

void view_draw(struct view *view, unsigned int bpp, unsigned int line)
{

    panel_draw(&view->panel, bpp, line);

    if (!view->active)
        return;

    if (view->windows.head)
    {

        struct list_item *current;

        for (current = view->windows.head; current; current = current->next)
        {

            struct window *window = current->data;

            window_draw(window, bpp, line);

        }

    }

    else
    {

        if (line < view->body.y || line >= view->body.y + view->body.h)
            return;

        fill(bpp, WM_COLOR_BODY, view->body.x, view->body.w);

    }

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

void view_init(struct view *view, struct box *screen, struct box *menu, unsigned int num, unsigned int total)
{

    list_inititem(&view->item, view);
    list_init(&view->windows);
    panel_init(&view->panel);
    box_setsize(&view->body, screen->x, screen->y + menu->h, screen->w, screen->h - menu->h);
    box_setsize(&view->panel.size, menu->x + num * menu->w / total, menu->y, menu->w / total, menu->h);

    view->center = screen->w / 2;
    view->active = 0;
    view->windowfocus = 0;

}

