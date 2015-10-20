#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"
#include "text.h"
#include "panel.h"
#include "window.h"
#include "client.h"
#include "view.h"

void view_activate(struct view *view)
{

    struct list_item *current;

    view->panel.header.active = 1;
    view->panel.base.modified = 1;
    view->number.header.type = TEXT_TYPE_HIGHLIGHT;
    view->number.base.modified = 1;

    for (current = view->clients.head; current; current = current->next)
    {

        struct client *client = current->data;

        client->window.base.modified = 1;
        client->window.base.header.z = 1;

    }

}

void view_deactivate(struct view *view)
{

    struct list_item *current;

    view->panel.header.active = 0;
    view->panel.base.modified = 1;
    view->number.header.type = TEXT_TYPE_NORMAL;
    view->number.base.modified = 1;

    for (current = view->clients.head; current; current = current->next)
    {

        struct client *client = current->data;

        client->window.base.modified = 1;
        client->window.base.header.z = 0;

    }

}

void view_init(struct view *view, unsigned int num)
{

    list_inititem(&view->item, view);
    panel_init(&view->panel);
    text_init(&view->number, TEXT_TYPE_NORMAL);
    text_assign(&view->number, 1, "12345678" + num);

    view->clientfocus = 0;
    view->panel.base.modified = 1;
    view->number.base.modified = 1;

}

