#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"
#include "draw.h"
#include "glyph.h"
#include "panel.h"
#include "window.h"
#include "client.h"
#include "view.h"

void view_activate(struct view *view)
{

    struct list_item *current;

    panel_activate(&view->panel);

    for (current = view->clients.head; current; current = current->next)
    {

        struct client *client = current->data;

        client->window.base.visible = 1;

    }

}

void view_deactivate(struct view *view)
{

    struct list_item *current;

    panel_deactivate(&view->panel);

    for (current = view->clients.head; current; current = current->next)
    {

        struct client *client = current->data;

        client->window.base.visible = 0;

    }

}

void view_init(struct view *view, struct box *screen, struct box *menu, unsigned int num, unsigned int total, char *text)
{

    list_inititem(&view->item, view);
    panel_init(&view->panel);
    glyph_init(&view->number, text);
    box_setsize(&view->body, screen->x, screen->y + menu->h, screen->w, screen->h - menu->h);
    box_setsize(&view->panel.base.size, menu->x + num * menu->w / total, menu->y, menu->w / total, menu->h);

    view->center = screen->w / 2;
    view->clientfocus = 0;
    view->panel.base.visible = 1;

}

