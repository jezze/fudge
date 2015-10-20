#include <abi.h>
#include <fudge.h>
#include <video/video.h>
#include "box.h"
#include "renderable.h"
#include "mouse.h"
#include "text.h"
#include "panel.h"
#include "window.h"
#include "send.h"

#define CLIENTS                         64
#define VIEWS                           8

static struct client
{

    struct list_item item;
    struct window window;
    unsigned int source;

} client[CLIENTS];

static struct view
{

    struct list_item item;
    struct list clients;
    unsigned int center;
    struct panel panel;
    struct text number;
    struct client *clientfocus;

} view[VIEWS];

static struct mouse mouse;
static struct list clients;
static struct ctrl_videosettings oldsettings;
static struct ctrl_videosettings settings;
static struct box screen;
static struct box menu;
static struct box body;
static unsigned char databuffer[FUDGE_BSIZE];
static unsigned int datacount;

static void writerenderable(unsigned int source, struct renderable *renderable)
{

    renderable->source = source;

    datacount += memory_write(databuffer, FUDGE_BSIZE, renderable, sizeof (struct renderable), datacount);

}

static void writepanel(unsigned int source, struct panel *panel)
{

    writerenderable(source, &panel->base);

    datacount += memory_write(databuffer, FUDGE_BSIZE, &panel->header, sizeof (struct panel_header), datacount);

}

static void writetext(unsigned int source, struct text *text, unsigned int count, void *buffer)
{

    writerenderable(source, &text->base);

    datacount += memory_write(databuffer, FUDGE_BSIZE, &text->header, sizeof (struct text_header), datacount);
    datacount += memory_write(databuffer, FUDGE_BSIZE, buffer, count, datacount);

}

static void writewindow(unsigned int source, struct window *window)
{

    writerenderable(source, &window->base);

    datacount += memory_write(databuffer, FUDGE_BSIZE, &window->header, sizeof (struct window_header), datacount);

}

static void flush()
{

    if (datacount)
    {

        call_write(CALL_PO, datacount, databuffer);

        datacount = 0;

    }

}

static void activateclient(unsigned int source, struct client *client)
{

    client->window.header.active = 1;

    writewindow(source, &client->window);

}

static void deactivateclient(unsigned int source, struct client *client)
{

    client->window.header.active = 0;

    writewindow(source, &client->window);

}

static void arrangeclients(unsigned int source, struct view *view, struct box *body)
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

        box_setsize(&client->window.base.size, body->x, body->y, body->w, body->h);
        writewindow(source, &client->window);

        send_wmmapnotify(CALL_L2, client->source, client->window.base.size.x, client->window.base.size.y, client->window.base.size.w, client->window.base.size.h);
        send_wmexpose(CALL_L2, client->source, client->window.base.size.x, client->window.base.size.y, client->window.base.size.w, client->window.base.size.h);

        break;

    default:
        y = body->y;
        h = body->h / (view->clients.count - 1);
        client = view->clients.tail->data;

        box_setsize(&client->window.base.size, body->x, body->y, view->center, body->h);
        writewindow(source, &client->window);

        send_wmmapnotify(CALL_L2, client->source, client->window.base.size.x, client->window.base.size.y, client->window.base.size.w, client->window.base.size.h);
        send_wmexpose(CALL_L2, client->source, client->window.base.size.x, client->window.base.size.y, client->window.base.size.w, client->window.base.size.h);

        for (current = view->clients.tail->prev; current; current = current->prev)
        {

            client = current->data;

            box_setsize(&client->window.base.size, body->x + view->center, y, body->w - view->center, h);
            writewindow(source, &client->window);

            send_wmmapnotify(CALL_L2, client->source, client->window.base.size.x, client->window.base.size.y, client->window.base.size.w, client->window.base.size.h);
            send_wmexpose(CALL_L2, client->source, client->window.base.size.x, client->window.base.size.y, client->window.base.size.w, client->window.base.size.h);

            y += h;

        }

        break;

    }

}

static struct client *focusclient(unsigned int source, struct client *focus, struct client *new)
{

    if (focus)
        deactivateclient(source, focus);

    if (new)
        activateclient(source, new);

    return new;

}

static struct client *nextclient(unsigned int source, struct client *focus, struct client *head)
{

    return focusclient(source, focus, (focus && focus->item.next) ? focus->item.next->data : head);

}

static struct client *prevclient(unsigned int source, struct client *focus, struct client *tail)
{

    return focusclient(source, focus, (focus && focus->item.prev) ? focus->item.prev->data : tail);

}

static struct client *findclient(struct view *view, unsigned int x, unsigned int y)
{

    struct list_item *current;

    for (current = view->clients.head; current; current = current->next)
    {

        struct client *client = current->data;

        if (box_isinside(&client->window.base.size, x, y))
            return client;

    }

    return 0;

}

static void mapclient(unsigned int source, struct view *view, unsigned int newsource)
{

    struct client *client;

    if (!clients.head)
        return;

    client = clients.head->data;
    client->source = newsource;

    list_move(&view->clients, &clients, &client->item);

    view->clientfocus = focusclient(source, view->clientfocus, client);

}

static void unmapclient(unsigned int source, struct view *view)
{

    if (!view->clientfocus)
        return;

    view->clientfocus->window.base.z = 0;

    flush();

    list_move(&clients, &view->clients, &view->clientfocus->item);

    view->clientfocus = focusclient(source, view->clientfocus, (view->clients.tail) ? view->clients.tail->data : 0);

}

static void unmapall(unsigned int source)
{

    unsigned int i;

    for (i = 0; i < VIEWS; i++)
    {

        while (view[i].clientfocus)
        {

            send_wmunmap(CALL_L2, view[i].clientfocus->source);
            unmapclient(source, &view[i]);

        }

    }

}

static void activateview(unsigned int source, struct view *view)
{

    struct list_item *current;

    view->panel.header.active = 1;
    view->number.header.type = TEXT_TYPE_HIGHLIGHT;

    writepanel(source, &view->panel);
    writetext(source, &view->number, view->number.count, view->number.string);

    for (current = view->clients.head; current; current = current->next)
    {

        struct client *client = current->data;

        client->window.base.z = 1;

        writewindow(source, &client->window);

    }

}

static void deactivateview(unsigned int source, struct view *view)
{

    struct list_item *current;

    view->panel.header.active = 0;
    view->number.header.type = TEXT_TYPE_NORMAL;

    writepanel(source, &view->panel);
    writetext(source, &view->number, view->number.count, view->number.string);

    for (current = view->clients.head; current; current = current->next)
    {

        struct client *client = current->data;

        client->window.base.z = 0;

        writewindow(source, &client->window);

    }

}

static struct view *focusview(unsigned int source, struct view *focus, struct view *new)
{

    if (focus)
        deactivateview(source, focus);

    if (new)
        activateview(source, new);

    return new;

}

static struct view *findview(unsigned int x, unsigned int y)
{

    unsigned int i;

    for (i = 0; i < VIEWS; i++)
    {

        if (box_isinside(&view[i].panel.base.size, x, y))
            return &view[i];

    }

    return 0;

}

static void setupclients(void)
{

    unsigned int i;

    for (i = 0; i < CLIENTS; i++)
    {

        list_inititem(&client[i].item, &client[i]);
        window_init(&client[i].window);
        list_add(&clients, &client[i].item);

    }

}

static void setupviews(void)
{

    unsigned int i;

    for (i = 0; i < VIEWS; i++)
    {

        list_inititem(&view[i].item, &view[i]);
        panel_init(&view[i].panel);
        text_init(&view[i].number, TEXT_TYPE_NORMAL);
        text_assign(&view[i].number, 1, "12345678" + i);

        view[i].clientfocus = 0;

    }

}

static void setupmouse(void)
{

    mouse_init(&mouse);

}

static void setviewsize(struct box *menu, struct box *body)
{

    unsigned int i;

    for (i = 0; i < VIEWS; i++)
    {

        view[i].center = body->w / 2;

        box_setsize(&view[i].panel.base.size, menu->x + i * menu->w / VIEWS, menu->y, menu->w / VIEWS, menu->h);
        box_setsize(&view[i].number.base.size, view[i].panel.base.size.x + 8, view[i].panel.base.size.y + 8, view[i].panel.base.size.w, 16);

    }

}

static void expose(unsigned int source, struct view *viewfocus, struct box *bb)
{

    struct list_item *current;
    unsigned int i;

    for (i = 0; i < VIEWS; i++)
    {

        writepanel(source, &view[i].panel);
        writetext(source, &view[i].number, view[i].number.count, view[i].number.string);

    }

    for (current = view->clients.head; current; current = current->next)
    {

        struct client *client = current->data;

        writewindow(source, &client->window);

    }

    writerenderable(source, &mouse.base);

}

void main(void)
{

    union event event;
    unsigned int count, quit = 0;
    struct view *viewfocus = &view[0];
    unsigned int source = 0;

    setupclients();
    setupviews();
    setupmouse();
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
                viewfocus = focusview(source, viewfocus, &view[event.keypress.scancode - 0x02]);

                flush();

                break;

            case 0x10:
                if (viewfocus->clientfocus)
                {

                    send_wmunmap(CALL_L2, viewfocus->clientfocus->source);
                    unmapclient(source, viewfocus);
                    arrangeclients(source, viewfocus, &body);
                    flush();

                }

                break;

            case 0x19:
                call_walk(CALL_CP, CALL_PR, 9, "bin/wnull");
                call_spawn();

                break;

            case 0x23:
                viewfocus->center -= (body.w / 32);

                arrangeclients(source, viewfocus, &body);
                flush();

                break;

            case 0x24:
                viewfocus->clientfocus = nextclient(source, viewfocus->clientfocus, viewfocus->clients.head ? viewfocus->clients.head->data : 0);

                flush();

                break;

            case 0x25:
                viewfocus->clientfocus = prevclient(source, viewfocus->clientfocus, viewfocus->clients.tail ? viewfocus->clients.tail->data : 0);

                flush();

                break;

            case 0x26:
                viewfocus->center += (body.w / 32);

                arrangeclients(source, viewfocus, &body);
                flush();

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

                    struct view *view = findview(mouse.base.size.x, mouse.base.size.y);
                    struct client *client = findclient(viewfocus, mouse.base.size.x, mouse.base.size.y);

                    if (view && view != viewfocus)
                    {

                        viewfocus = focusview(source, viewfocus, view);

                        flush();

                    }

                    if (client && client != viewfocus->clientfocus)
                    {

                        viewfocus->clientfocus = focusclient(source, viewfocus->clientfocus, client);

                        flush();

                    }

                    break;

                }

            }

            break;

        case EVENT_MOUSEMOVE:
            mouse.base.size.x += event.mousemove.relx;
            mouse.base.size.y -= event.mousemove.rely;

            if (event.mousemove.relx > 0 && mouse.base.size.x >= screen.w)
                mouse.base.size.x = screen.w - 1;

            if (event.mousemove.relx < 0 && mouse.base.size.x >= screen.w)
                mouse.base.size.x = 0;

            if (event.mousemove.rely < 0 && mouse.base.size.y >= screen.h)
                mouse.base.size.y = screen.h - 1;

            if (event.mousemove.rely > 0 && mouse.base.size.y >= screen.h)
                mouse.base.size.y = 0;

            writerenderable(source, &mouse.base);
            flush();

            break;

        case EVENT_WMMAP:
            if (event.header.source == event.header.destination)
            {

                ctrl_setvideosettings(&settings, 1920, 1080, 32);
                video_getmode(CALL_L0, &oldsettings);
                video_setmode(CALL_L0, &settings);
                video_getmode(CALL_L0, &settings);
                send_wmmapnotify(CALL_L2, event.header.source, 0, 0, settings.w, settings.h);
                send_wmexpose(CALL_L2, event.header.source, 0, 0, settings.w, settings.h);

            }
            
            else
            {

                mapclient(source, viewfocus, event.header.source);
                arrangeclients(source, viewfocus, &body);
                send_wmmapnotify(CALL_L2, event.header.source, viewfocus->clientfocus->window.base.size.x, viewfocus->clientfocus->window.base.size.y, viewfocus->clientfocus->window.base.size.w, viewfocus->clientfocus->window.base.size.h);
                send_wmexpose(CALL_L2, event.header.source, viewfocus->clientfocus->window.base.size.x, viewfocus->clientfocus->window.base.size.y, viewfocus->clientfocus->window.base.size.w, viewfocus->clientfocus->window.base.size.h);
                flush();

            }

            break;

        case EVENT_WMMAPNOTIFY:
            source = event.header.destination;

            box_setsize(&screen, event.wmmapnotify.x, event.wmmapnotify.y, event.wmmapnotify.w, event.wmmapnotify.h);
            box_setsize(&menu, screen.x, screen.y, screen.w, 32);
            box_setsize(&body, screen.x, screen.y + 32, screen.w, screen.h - 32);
            setviewsize(&menu, &body);
            box_setsize(&mouse.base.size, screen.x + screen.w / 4, screen.y + screen.h / 4, 24, 24);
            activateview(source, viewfocus);

            break;

        case EVENT_WMEXPOSE:
            expose(source, viewfocus, &screen);
            flush();

            break;

        case EVENT_WMUNMAP:
            if (event.header.source == event.header.destination)
                video_setmode(CALL_L0, &oldsettings);

            unmapall(source);

            quit = 1;

            break;

        }

        if (quit)
            break;

    }

    call_close(CALL_L1);
    call_close(CALL_PO);

}

