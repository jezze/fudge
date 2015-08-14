#include <abi.h>
#include <fudge.h>
#include <video/video.h>
#include <pcf/lib.h>
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

static void arrangeclients(struct view *view)
{

    unsigned int count = list_count(&view->clients);
    struct list_item *current = view->clients.tail;
    struct client *client;
    unsigned int a, i;

    if (!count)
        return;

    client = current->data;

    if (count == 1)
    {

        box_setsize(&client->window.base.size, view->body.x, view->body.y, view->body.w, view->body.h);

        return;

    }

    box_setsize(&client->window.base.size, view->body.x, view->body.y, view->center, view->body.h);

    a = view->body.h / (count - 1);
    i = 0;

    for (current = current->prev; current; current = current->prev)
    {

        client = current->data;

        box_setsize(&client->window.base.size, view->body.x + view->center, view->body.y + i * a, view->body.w - view->center, a);

        i++;

    }

}

static struct client *focusclient(struct client *old, struct client *new)
{

    if (old)
        window_deactivate(&old->window);

    if (new)
        window_activate(&new->window);

    return new;

}

static void nextclient(struct view *view)
{

    if (!view->clientfocus)
        return;

    if (view->clientfocus->item.next)
        view->clientfocus = focusclient(view->clientfocus, view->clientfocus->item.next->data);
    else
        view->clientfocus = focusclient(view->clientfocus, view->clients.head->data);

}

static void prevclient(struct view *view)
{

    if (!view->clientfocus)
        return;

    if (view->clientfocus->item.prev)
        view->clientfocus = focusclient(view->clientfocus, view->clientfocus->item.prev->data);
    else
        view->clientfocus = focusclient(view->clientfocus, view->clients.tail->data);

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

    if (view->clients.tail)
        view->clientfocus = focusclient(view->clientfocus, view->clients.tail->data);
    else
        view->clientfocus = focusclient(view->clientfocus, 0);

}

static struct view *focusview(struct view *old, struct view *new)
{

    if (old)
        view_deactivate(old);

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

static void pollevent(struct ctrl_videosettings *settings, struct box *screen)
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
                    arrangeclients(viewfocus);
                    draw(settings, &viewfocus->body);

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
                viewfocus->center -= (viewfocus->body.w / 32);

                arrangeclients(viewfocus);
                draw(settings, &viewfocus->body);

                break;

            case 0x24:
                nextclient(viewfocus);
                draw(settings, &viewfocus->body);

                break;

            case 0x25:
                prevclient(viewfocus);
                draw(settings, &viewfocus->body);

                break;

            case 0x26:
                viewfocus->center += (viewfocus->body.w / 32);

                arrangeclients(viewfocus);
                draw(settings, &viewfocus->body);

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

                        draw(settings, &viewfocus->body);

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
            arrangeclients(viewfocus);
            draw(settings, &viewfocus->body);

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

static struct view *setupviews(struct box *screen)
{

    struct box menu;
    unsigned int i;
    char *numbers = "12345678";

    box_setsize(&menu, screen->x, screen->y, screen->w, 32);

    for (i = 0; i < VIEWS; i++)
    {

        view_init(&view[i], screen, &menu, i, VIEWS, numbers + i);
        list_add(&views, &view[i].item);
        list_add(&renderables, &view[i].panel.base.item);

    }

    return views.head->data;

}

static void setupmouse(struct box *screen)
{

    mouse_init(&mouse, screen);
    list_add(&renderables, &mouse.image.base.item);

    mouse.image.base.visible = 1;

}

static void setupfont()
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

    ctrl_setvideosettings(&settings, 1024, 768, 32);
    video_getmode(CALL_L0, &oldsettings);
    video_setmode(CALL_L0, &settings);
    box_setsize(&screen, 0, 0, settings.w, settings.h);
    setupfont();
    setupclients();

    viewfocus = setupviews(&screen);

    view_activate(viewfocus);
    setupmouse(&screen);
    draw_init();
    draw(&settings, &screen);
    pollevent(&settings, &screen);
    video_setmode(CALL_L0, &oldsettings);

}

