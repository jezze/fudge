#include <abi.h>
#include <fudge.h>
#include <video/video.h>
#include <format/pcf.h>
#include "box.h"
#include "renderable.h"
#include "draw.h"
#include "image.h"
#include "mouse.h"
#include "glyph.h"
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
static struct list views;
static struct view *viewfocus;
static struct list renderables;
static unsigned char fontdata[0x8000];

static void draw(struct ctrl_videosettings *settings, struct box *bb)
{

    unsigned int line;

    video_open(CALL_L0);

    for (line = bb->y; line < bb->y + bb->h; line++)
    {

        struct list_item *current;

        draw_fill(settings->bpp, WM_COLOR_BODY, bb->x, bb->w);

        for (current = renderables.head; current; current = current->next)
        {

            struct renderable *renderable = current->data;

            renderable_render(renderable, settings, line);

        }

        draw_flush(settings->w * line, settings->bpp, bb->x, bb->w);

    }

    video_close(CALL_L0);

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

        box_setsize(&client->window.base.size, body->x, body->y, body->w, body->h);

        break;

    default:
        client = view->clients.tail->data;

        box_setsize(&client->window.base.size, body->x, body->y, view->center, body->h);

        y = 0;
        h = body->h / (view->clients.count - 1);

        for (current = view->clients.tail->prev; current; current = current->prev)
        {

            client = current->data;

            box_setsize(&client->window.base.size, body->x + view->center, body->y + y * h, body->w - view->center, h);

            y++;

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

    if (!focus)
        return 0;

    return focusclient(focus, (focus->item.next) ? focus->item.next->data : head);

}

static struct client *prevclient(struct client *focus, struct client *tail)
{

    if (!focus)
        return 0;

    return focusclient(focus, (focus->item.prev) ? focus->item.prev->data : tail);

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

static void mapclient(struct view *view, unsigned int source)
{

    struct client *client;

    if (!clients.head)
        return;

    client = clients.head->data;
    client->source = source;

    list_move(&view->clients, &clients, &client->item);

    view->clientfocus = focusclient(view->clientfocus, client);
    view->clientfocus->window.base.visible = 1;

}

static void unmapclient(struct view *view)
{

    if (!view->clientfocus)
        return;

    view->clientfocus->window.base.visible = 0;

    list_move(&clients, &view->clients, &view->clientfocus->item);

    view->clientfocus = focusclient(view->clientfocus, (view->clients.tail) ? view->clients.tail->data : 0);

}

static struct view *focusview(struct view *focus, struct view *new)
{

    if (focus)
        view_deactivate(focus);

    if (new)
        view_activate(new);

    return new;

}

static struct view *findview(struct list *views, unsigned int x, unsigned int y)
{

    struct list_item *current;

    for (current = views->head; current; current = current->next)
    {

        struct view *view = current->data;

        if (box_isinside(&view->panel.base.size, x, y))
            return view;

    }

    return 0;

}

static void pollevent(struct ctrl_videosettings *settings, struct box *screen, struct box *menu, struct box *body)
{

    union event event;
    unsigned int count, roff, quit = 0;
    struct box oldmouse;

    call_walk(CALL_L1, CALL_PR, 17, "system/event/poll");
    call_open(CALL_L1);

    for (roff = 0; (count = call_read(CALL_L1, roff, sizeof (struct event_header), 1, &event.header)); roff += count)
    {

        if (event.header.count)
            count += call_read(CALL_L1, roff + count, event.header.count, 1, event.data + sizeof (struct event_header));

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

                draw(settings, screen);

                break;

            case 0x10:
                if (viewfocus->clientfocus)
                {

                    send_wmunmap(viewfocus->clientfocus->source);
                    unmapclient(viewfocus);
                    arrangeclients(viewfocus, body);
                    draw(settings, body);

                }

                break;

            case 0x11:
                box_setsize(&oldmouse, mouse.image.base.size.x, mouse.image.base.size.y, mouse.image.base.size.w, mouse.image.base.size.h);

                mouse.image.base.size.y -= 4;

                if (mouse.image.base.size.y >= screen->h)
                    mouse.image.base.size.y = 0;

                draw(settings, &oldmouse);
                draw(settings, &mouse.image.base.size);

                break;

            case 0x19:
                call_walk(CALL_CP, CALL_PR, 9, "bin/wnull");
                call_spawn();

                break;

            case 0x1E:
                box_setsize(&oldmouse, mouse.image.base.size.x, mouse.image.base.size.y, mouse.image.base.size.w, mouse.image.base.size.h);

                mouse.image.base.size.x -= 4;

                if (mouse.image.base.size.x >= screen->w)
                    mouse.image.base.size.x = 0;

                draw(settings, &oldmouse);
                draw(settings, &mouse.image.base.size);

                break;

            case 0x1F:
                box_setsize(&oldmouse, mouse.image.base.size.x, mouse.image.base.size.y, mouse.image.base.size.w, mouse.image.base.size.h);

                mouse.image.base.size.y += 4;

                if (mouse.image.base.size.y + mouse.image.base.size.h > screen->h)
                    mouse.image.base.size.y = screen->h - mouse.image.base.size.h;

                draw(settings, &oldmouse);
                draw(settings, &mouse.image.base.size);

                break;

            case 0x20:
                box_setsize(&oldmouse, mouse.image.base.size.x, mouse.image.base.size.y, mouse.image.base.size.w, mouse.image.base.size.h);

                mouse.image.base.size.x += 4;

                if (mouse.image.base.size.x + mouse.image.base.size.w > screen->w)
                    mouse.image.base.size.x = screen->w - mouse.image.base.size.w;

                draw(settings, &oldmouse);
                draw(settings, &mouse.image.base.size);

                break;

            case 0x23:
                viewfocus->center -= (body->w / 32);

                arrangeclients(viewfocus, body);
                draw(settings, body);

                break;

            case 0x24:
                viewfocus->clientfocus = nextclient(viewfocus->clientfocus, viewfocus->clients.head->data);
                draw(settings, body);

                break;

            case 0x25:
                viewfocus->clientfocus = prevclient(viewfocus->clientfocus, viewfocus->clients.tail->data);
                draw(settings, body);

                break;

            case 0x26:
                viewfocus->center += (body->w / 32);

                arrangeclients(viewfocus, body);
                draw(settings, body);

                break;

            case 0x2C:
                quit = 1;

                break;

            }

        case EVENT_MOUSEPRESS:
            switch (event.mousepress.button)
            {

            case 0x01:
                {

                    struct view *view = findview(&views, mouse.image.base.size.x, mouse.image.base.size.y);
                    struct client *client = findclient(viewfocus, mouse.image.base.size.x, mouse.image.base.size.y);

                    if (view && view != viewfocus)
                    {

                        viewfocus = focusview(viewfocus, view);

                        draw(settings, screen);

                    }

                    if (client && client != viewfocus->clientfocus)
                    {

                        viewfocus->clientfocus = focusclient(viewfocus->clientfocus, client);

                        draw(settings, body);

                    }

                    break;

                }

            }

            break;

        case EVENT_MOUSEMOVE:
            box_setsize(&oldmouse, mouse.image.base.size.x, mouse.image.base.size.y, mouse.image.base.size.w, mouse.image.base.size.h);

            mouse.image.base.size.x += event.mousemove.relx;
            mouse.image.base.size.y -= event.mousemove.rely;

            if (event.mousemove.relx > 0 && mouse.image.base.size.x + mouse.image.base.size.w > screen->w)
                mouse.image.base.size.x = screen->w - mouse.image.base.size.w;

            if (event.mousemove.relx < 0 && mouse.image.base.size.x >= screen->w)
                mouse.image.base.size.x = 0;

            if (event.mousemove.rely < 0 && mouse.image.base.size.y + mouse.image.base.size.h > screen->h)
                mouse.image.base.size.y = screen->h - mouse.image.base.size.h;

            if (event.mousemove.rely > 0 && mouse.image.base.size.y >= screen->h)
                mouse.image.base.size.y = 0;

            draw(settings, &oldmouse);
            draw(settings, &mouse.image.base.size);

            break;

        case EVENT_WMMAP:
            mapclient(viewfocus, event.header.source);
            arrangeclients(viewfocus, body);
            draw(settings, body);

            break;

        }

        if (quit)
            break;

    }

    call_close(CALL_L1);

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

static void setupviews(struct box *menu, struct box *body)
{

    unsigned int i;

    for (i = 0; i < VIEWS; i++)
    {

        view_init(&view[i], menu, body, i, VIEWS, fontdata);
        list_add(&views, &view[i].item);
        list_add(&renderables, &view[i].panel.base.item);
        list_add(&renderables, &view[i].number.base.item);

    }

}

static void setupmouse(struct box *screen)
{

    mouse_init(&mouse, screen);
    list_add(&renderables, &mouse.image.base.item);

    mouse.image.base.visible = 1;

}

static void setupfont(void)
{

    call_walk(CALL_L4, CALL_PR, 18, "share/ter-u16n.pcf");
    call_open(CALL_L4);
    call_read(CALL_L4, 0, 1, 0x8000, fontdata);
    call_close(CALL_L4);

}

void main(void)
{

    struct ctrl_videosettings oldsettings;
    struct ctrl_videosettings settings;
    struct box screen;
    struct box menu;
    struct box body;

    ctrl_setvideosettings(&settings, 1920, 1080, 32);
    video_getmode(CALL_L0, &oldsettings);
    video_setmode(CALL_L0, &settings);
    box_setsize(&screen, 0, 0, settings.w, settings.h);
    box_setsize(&menu, screen.x, screen.y, screen.w, 32);
    box_setsize(&body, screen.x, screen.y + 32, screen.w, screen.h - 32);
    setupfont();
    setupclients();
    setupviews(&menu, &body);

    viewfocus = views.head->data;

    view_activate(viewfocus);
    setupmouse(&screen);
    draw_init();
    draw(&settings, &screen);
    pollevent(&settings, &screen, &menu, &body);
    video_setmode(CALL_L0, &oldsettings);

}

