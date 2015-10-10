#include <abi.h>
#include <fudge.h>
#include <video/video.h>
#include "box.h"
#include "renderable.h"
#include "mouse.h"
#include "text.h"
#include "panel.h"
#include "window.h"
#include "client.h"
#include "view.h"
#include "send.h"

#define CLIENTS                         64
#define VIEWS                           8

static struct mouse mouse;
static struct client client[CLIENTS];
static struct list clients;
static struct view view[VIEWS];
static struct list renderables;
static struct ctrl_videosettings oldsettings;
static struct ctrl_videosettings settings;
static struct box screen;
static struct box menu;
static struct box body;

static unsigned int writerenderable(unsigned int source, void *buffer, unsigned int count, struct renderable *renderable)
{

    renderable->header.source = source;

    return memory_write(buffer, FUDGE_BSIZE, &renderable->header, sizeof (struct renderable_header), count);

}

static unsigned int writepanel(void *buffer, unsigned int count, struct panel *panel)
{

    return memory_write(buffer, FUDGE_BSIZE, &panel->header, sizeof (struct panel_header), count);

}

static unsigned int writetext(void *buffer, unsigned int count, struct text *text)
{

    return memory_write(buffer, FUDGE_BSIZE, &text->header, sizeof (struct text_header), count);

}

static unsigned int writewindow(void *buffer, unsigned int count, struct window *window)
{

    return memory_write(buffer, FUDGE_BSIZE, &window->header, sizeof (struct window_header), count);

}

static unsigned int writepayload(void *buffer, unsigned int count, void *payload, unsigned int payloadcount)
{

    return memory_write(buffer, FUDGE_BSIZE, payload, payloadcount, count);

}

static void render(unsigned int source, struct box *bb)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count = 0;
    struct list_item *current;

    for (current = renderables.head; current; current = current->next)
    {

        struct renderable *renderable = current->data;
        struct text *text;

        if (!renderable->modified)
            continue;

        count += writerenderable(source, buffer, count, renderable);

        switch (renderable->header.type)
        {

        case RENDERABLE_TYPE_PANEL:
            count += writepanel(buffer, count, current->data);

            break;

        case RENDERABLE_TYPE_TEXT:
            text = current->data;

            count += writetext(buffer, count, text);
            count += writepayload(buffer, count, text->string, text->count);

            break;

        case RENDERABLE_TYPE_WINDOW:
            count += writewindow(buffer, count, current->data);

            break;

        }

        renderable->modified = 0;

    }

    call_write(CALL_PO, count, buffer);

}

static void arrangeclients(struct view *view, struct box *body)
{

    struct list_item *current;
    struct client *client;
    unsigned int y;
    unsigned int h;

    switch (view->clients.count)
    {

    case 0:
        break;

    case 1:
        client = view->clients.tail->data;
        client->window.base.modified = 1;

        box_setsize(&client->window.base.header.size, body->x, body->y, body->w, body->h);
        send_wmready(CALL_L2, client->source, client->window.base.header.size.x, client->window.base.header.size.y, client->window.base.header.size.w, client->window.base.header.size.h);
        send_wmexpose(CALL_L2, client->source, client->window.base.header.size.x, client->window.base.header.size.y, client->window.base.header.size.w, client->window.base.header.size.h);

        break;

    default:
        y = body->y;
        h = body->h / (view->clients.count - 1);
        client = view->clients.tail->data;
        client->window.base.modified = 1;

        box_setsize(&client->window.base.header.size, body->x, body->y, view->center, body->h);
        send_wmready(CALL_L2, client->source, client->window.base.header.size.x, client->window.base.header.size.y, client->window.base.header.size.w, client->window.base.header.size.h);
        send_wmexpose(CALL_L2, client->source, client->window.base.header.size.x, client->window.base.header.size.y, client->window.base.header.size.w, client->window.base.header.size.h);

        for (current = view->clients.tail->prev; current; current = current->prev)
        {

            client = current->data;
            client->window.base.modified = 1;

            box_setsize(&client->window.base.header.size, body->x + view->center, y, body->w - view->center, h);
            send_wmready(CALL_L2, client->source, client->window.base.header.size.x, client->window.base.header.size.y, client->window.base.header.size.w, client->window.base.header.size.h);
            send_wmexpose(CALL_L2, client->source, client->window.base.header.size.x, client->window.base.header.size.y, client->window.base.header.size.w, client->window.base.header.size.h);

            y += h;

        }

        break;

    }

}

static struct client *focusclient(struct client *focus, struct client *new)
{

    if (focus)
        client_deactivate(focus);

    if (new)
        client_activate(new);

    return new;

}

static struct client *nextclient(struct client *focus, struct client *head)
{

    return focusclient(focus, (focus && focus->item.next) ? focus->item.next->data : head);

}

static struct client *prevclient(struct client *focus, struct client *tail)
{

    return focusclient(focus, (focus && focus->item.prev) ? focus->item.prev->data : tail);

}

static struct client *findclient(struct view *view, unsigned int x, unsigned int y)
{

    struct list_item *current;

    for (current = view->clients.head; current; current = current->next)
    {

        struct client *client = current->data;

        if (box_isinside(&client->window.base.header.size, x, y))
            return client;

    }

    return 0;

}

static void mapclient(struct view *view, unsigned int source)
{

    struct client *client;

    if (!clients.head)
        return;

    client = clients.head->data;
    client->source = source;

    list_move(&view->clients, &clients, &client->item);

    view->clientfocus = focusclient(view->clientfocus, client);

}

static void unmapclient(struct view *view)
{

    if (!view->clientfocus)
        return;

    list_move(&clients, &view->clients, &view->clientfocus->item);

    view->clientfocus = focusclient(view->clientfocus, (view->clients.tail) ? view->clients.tail->data : 0);

}

static void unmapall(void)
{

    unsigned int i;

    for (i = 0; i < VIEWS; i++)
    {

        while (view[i].clientfocus)
        {

            send_wmunmap(CALL_L2, view[i].clientfocus->source);
            unmapclient(&view[i]);

        }

    }

}

static struct view *focusview(struct view *focus, struct view *new)
{

    if (focus)
        view_deactivate(focus);

    if (new)
        view_activate(new);

    return new;

}

static struct view *findview(unsigned int x, unsigned int y)
{

    unsigned int i;

    for (i = 0; i < VIEWS; i++)
    {

        if (box_isinside(&view[i].panel.base.header.size, x, y))
            return &view[i];

    }

    return 0;

}

static void setupclients(void)
{

    unsigned int i;

    for (i = 0; i < CLIENTS; i++)
    {

        client_init(&client[i]);
        list_add(&clients, &client[i].item);
        list_add(&renderables, &client[i].window.base.item);

    }

}

static void setupviews(void)
{

    unsigned int i;

    for (i = 0; i < VIEWS; i++)
    {

        view_init(&view[i], i);
        list_add(&renderables, &view[i].panel.base.item);
        list_add(&renderables, &view[i].number.base.item);

    }

}

static void setupmouse(void)
{

    mouse_init(&mouse);
    list_add(&renderables, &mouse.base.item);

}

static void setviewsize(struct box *menu, struct box *body)
{

    unsigned int i;

    for (i = 0; i < VIEWS; i++)
    {

        view[i].center = body->w / 2;

        box_setsize(&view[i].panel.base.header.size, menu->x + i * menu->w / VIEWS, menu->y, menu->w / VIEWS, menu->h);
        box_setsize(&view[i].number.base.header.size, view[i].panel.base.header.size.x + 8, view[i].panel.base.header.size.y + 8, view[i].panel.base.header.size.w, 16);

    }

}

void main(void)
{

    union event event;
    struct box oldmouse;
    unsigned int count, quit = 0;
    struct view *viewfocus = &view[0];
    unsigned int source = 0;

    setupclients();
    setupviews();
    setupmouse();
    view_activate(viewfocus);
    call_open(CALL_PO);
    call_walk(CALL_L1, CALL_PR, 17, "system/event/poll");
    call_open(CALL_L1);
    send_wmmap(CALL_L2, 0);

    while ((count = call_read(CALL_L1, sizeof (struct event_header), &event.header)))
    {

        if (event.header.count)
            call_read(CALL_L1, event.header.count, event.data + sizeof (struct event_header));

        switch (event.header.type)
        {

        case EVENT_KEYPRESS:
            switch (event.keypress.scancode)
            {

            case 0x02:
            case 0x03:
            case 0x04:
            case 0x05:
            case 0x06:
            case 0x07:
            case 0x08:
            case 0x09:
                viewfocus = focusview(viewfocus, &view[event.keypress.scancode - 0x02]);

                render(source, &screen);

                break;

            case 0x10:
                if (viewfocus->clientfocus)
                {

                    send_wmunmap(CALL_L2, viewfocus->clientfocus->source);
                    unmapclient(viewfocus);
                    arrangeclients(viewfocus, &body);
                    render(source, &body);

                }

                break;

            case 0x19:
                call_walk(CALL_CP, CALL_PR, 9, "bin/wnull");
                call_spawn();

                break;

            case 0x23:
                viewfocus->center -= (body.w / 32);

                arrangeclients(viewfocus, &body);
                render(source, &body);

                break;

            case 0x24:
                viewfocus->clientfocus = nextclient(viewfocus->clientfocus, viewfocus->clients.head ? viewfocus->clients.head->data : 0);

                render(source, &body);

                break;

            case 0x25:
                viewfocus->clientfocus = prevclient(viewfocus->clientfocus, viewfocus->clients.tail ? viewfocus->clients.tail->data : 0);

                render(source, &body);

                break;

            case 0x26:
                viewfocus->center += (body.w / 32);

                arrangeclients(viewfocus, &body);
                render(source, &body);

                break;

            case 0x2C:
                send_wmunmap(CALL_L2, 0);

                break;

            }

        case EVENT_MOUSEPRESS:
            switch (event.mousepress.button)
            {

            case 0x01:
                {

                    struct view *view = findview(mouse.base.header.size.x, mouse.base.header.size.y);
                    struct client *client = findclient(viewfocus, mouse.base.header.size.x, mouse.base.header.size.y);

                    if (view && view != viewfocus)
                    {

                        viewfocus = focusview(viewfocus, view);

                        render(source, &screen);

                    }

                    if (client && client != viewfocus->clientfocus)
                    {

                        viewfocus->clientfocus = focusclient(viewfocus->clientfocus, client);

                        render(source, &body);

                    }

                    break;

                }

            }

            break;

        case EVENT_MOUSEMOVE:
            box_setsize(&oldmouse, mouse.base.header.size.x, mouse.base.header.size.y, mouse.base.header.size.w, mouse.base.header.size.h);

            mouse.base.header.size.x += event.mousemove.relx;
            mouse.base.header.size.y -= event.mousemove.rely;

            if (event.mousemove.relx > 0 && mouse.base.header.size.x >= screen.w)
                mouse.base.header.size.x = screen.w - 1;

            if (event.mousemove.relx < 0 && mouse.base.header.size.x >= screen.w)
                mouse.base.header.size.x = 0;

            if (event.mousemove.rely < 0 && mouse.base.header.size.y >= screen.h)
                mouse.base.header.size.y = screen.h - 1;

            if (event.mousemove.rely > 0 && mouse.base.header.size.y >= screen.h)
                mouse.base.header.size.y = 0;

            render(source, &oldmouse);
            render(source, &mouse.base.header.size);

            break;

        case EVENT_WMMAP:
            if (event.header.source == event.header.destination)
            {

                ctrl_setvideosettings(&settings, 1920, 1080, 32);
                video_getmode(CALL_L0, &oldsettings);
                video_setmode(CALL_L0, &settings);
                video_getmode(CALL_L0, &settings);
                send_wmready(CALL_L2, event.header.source, 0, 0, settings.w, settings.h);
                send_wmexpose(CALL_L2, event.header.source, 0, 0, settings.w, settings.h);

            }
            
            else
            {

                mapclient(viewfocus, event.header.source);
                arrangeclients(viewfocus, &body);
                send_wmready(CALL_L2, event.header.source, viewfocus->clientfocus->window.base.header.size.x, viewfocus->clientfocus->window.base.header.size.y, viewfocus->clientfocus->window.base.header.size.w, viewfocus->clientfocus->window.base.header.size.h);
                send_wmexpose(CALL_L2, event.header.source, viewfocus->clientfocus->window.base.header.size.x, viewfocus->clientfocus->window.base.header.size.y, viewfocus->clientfocus->window.base.header.size.w, viewfocus->clientfocus->window.base.header.size.h);
                render(source, &body);

            }

            break;

        case EVENT_WMREADY:
            source = event.header.destination;

            box_setsize(&screen, event.wmready.x, event.wmready.y, event.wmready.w, event.wmready.h);
            box_setsize(&menu, screen.x, screen.y, screen.w, 32);
            box_setsize(&body, screen.x, screen.y + 32, screen.w, screen.h - 32);
            setviewsize(&menu, &body);
            box_setsize(&mouse.base.header.size, screen.x + screen.w / 4, screen.y + screen.h / 4, 24, 24);

            break;

        case EVENT_WMEXPOSE:
            render(source, &screen);

            break;

        case EVENT_WMUNMAP:
            if (event.header.source == event.header.destination)
                video_setmode(CALL_L0, &oldsettings);

            unmapall();

            quit = 1;

            break;

        }

        if (quit)
            break;

    }

    call_close(CALL_L1);
    call_close(CALL_PO);

}

