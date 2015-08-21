#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"
#include "draw.h"
#include "text.h"
#include "panel.h"
#include "window.h"
#include "client.h"
#include "view.h"

void view_activate(struct view *view)
{

    struct list_item *current;

    view->panel.backgroundcolor = WM_COLOR_ACTIVEBACK;
    view->panel.framecolor = WM_COLOR_ACTIVEFRAME;
    view->number.color = WM_COLOR_TEXTLIGHT;

    for (current = view->clients.head; current; current = current->next)
    {

        struct client *client = current->data;

        client->window.base.visible = 1;

    }

}

void view_deactivate(struct view *view)
{

    struct list_item *current;

    view->panel.backgroundcolor = WM_COLOR_PASSIVEBACK;
    view->panel.framecolor = WM_COLOR_PASSIVEFRAME;
    view->number.color = WM_COLOR_TEXTDARK;

    for (current = view->clients.head; current; current = current->next)
    {

        struct client *client = current->data;

        client->window.base.visible = 0;

    }

}

void view_init(struct view *view, struct box *menu, struct box *body, unsigned int num, unsigned int total, unsigned char *fontdata)
{

    list_inititem(&view->item, view);
    panel_init(&view->panel);
    text_init(&view->number, WM_COLOR_TEXTDARK);
    text_assign(&view->number, fontdata, 1, "12345678" + num);
    box_setsize(&view->panel.base.size, menu->x + num * menu->w / total, menu->y, menu->w / total, menu->h);
    box_setsize(&view->number.base.size, view->panel.base.size.x + 8, view->panel.base.size.y + 8, view->panel.base.size.w, 16);

    view->center = body->w / 2;
    view->clientfocus = 0;
    view->panel.base.visible = 1;
    view->number.base.visible = 1;

}

