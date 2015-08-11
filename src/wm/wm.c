#include <abi.h>
#include <fudge.h>
#include <video/video.h>
#include "box.h"
#include "draw.h"
#include "image.h"
#include "mouse.h"
#include "panel.h"
#include "window.h"
#include "view.h"
#include "send.h"

#define WINDOWS                         64
#define VIEWS                           8

static struct mouse mouse;
static struct window window[WINDOWS];
static struct list windows;
static struct view view[VIEWS];
static struct list views;
static struct view *viewfocus;

static void spawn(void)
{

    call_walk(CALL_CP, CALL_PR, 9, "bin/wnull");
    call_spawn();

}

static void sendwmdrawall(struct box *bb)
{

    struct list_item *current;

    for (current = views.head; current; current = current->next)
    {

        struct view *view = current->data;
        struct list_item *current2;

        if (!view->active)
            continue;

        for (current2 = view->windows.head; current2; current2 = current2->next)
        {

            struct window *window = current2->data;

            send_wmdraw(window->source, bb->x, bb->y, bb->w, bb->h);

        }

    }

}

static void draw(struct ctrl_videosettings *settings, struct box *bb, unsigned int notify)
{

    unsigned int line;

    video_open();

    for (line = bb->y; line < bb->y + bb->h; line++)
    {

        struct list_item *current;

        draw_fill(settings->bpp, WM_COLOR_TRANSPARENT, bb->x, bb->w);

        for (current = views.head; current; current = current->next)
        {

            struct view *view = current->data;

            view_draw(view, settings, line);

        }

        draw_flush(settings->w * line, settings->bpp, bb->x, bb->w);

    }

    video_close();

    if (notify)
        sendwmdrawall(bb);

    video_open();

    for (line = bb->y; line < bb->y + bb->h; line++)
    {

        draw_fill(settings->bpp, WM_COLOR_TRANSPARENT, bb->x, bb->w);
        image_draw(&mouse.image, settings, line);
        draw_flush(settings->w * line, settings->bpp, bb->x, bb->w);

    }

    video_close();

}

static void arrangewindows(struct view *view)
{

    unsigned int count = list_count(&view->windows);
    struct list_item *current = view->windows.tail;
    struct window *window;
    unsigned int a, i;

    if (!count)
        return;

    window = current->data;

    if (count == 1)
    {

        box_setsize(&window->size, view->body.x, view->body.y, view->body.w, view->body.h);
        box_setsize(&window->screen, window->size.x + 3, window->size.y + 3, window->size.w - 6, window->size.h - 6);
        send_wmresize(window->source, window->screen.x, window->screen.y, window->screen.w, window->screen.h);

        return;

    }

    box_setsize(&window->size, view->body.x, view->body.y, view->center, view->body.h);
    box_setsize(&window->screen, window->size.x + 3, window->size.y + 3, window->size.w - 6, window->size.h - 6);
    send_wmresize(window->source, window->screen.x, window->screen.y, window->screen.w, window->screen.h);

    a = view->body.h / (count - 1);
    i = 0;

    for (current = current->prev; current; current = current->prev)
    {

        window = current->data;

        box_setsize(&window->size, view->body.x + view->center, view->body.y + i * a, view->body.w - view->center, a);
        box_setsize(&window->screen, window->size.x + 3, window->size.y + 3, window->size.w - 6, window->size.h - 6);
        send_wmresize(window->source, window->screen.x, window->screen.y, window->screen.w, window->screen.h);

        i++;

    }

}

static struct window *focuswindow(struct window *old, struct window *new)
{

    if (old)
        window_deactivate(old);

    if (new)
        window_activate(new);

    return new;

}

static void nextwindow(struct view *view)
{

    if (!view->windowfocus)
        return;

    if (view->windowfocus->item.next)
        view->windowfocus = focuswindow(view->windowfocus, view->windowfocus->item.next->data);
    else
        view->windowfocus = focuswindow(view->windowfocus, view->windows.head->data);

}

static void prevwindow(struct view *view)
{

    if (!view->windowfocus)
        return;

    if (view->windowfocus->item.prev)
        view->windowfocus = focuswindow(view->windowfocus, view->windowfocus->item.prev->data);
    else
        view->windowfocus = focuswindow(view->windowfocus, view->windows.tail->data);

}

static void mapwindow(struct view *view, unsigned int source)
{

    struct window *window;

    if (!windows.head)
        return;

    window = windows.head->data;
    window->source = source;

    list_move(&view->windows, &windows, &window->item);

    view->windowfocus = focuswindow(view->windowfocus, window);

}

static void unmapwindow(struct view *view)
{

    if (!view->windowfocus)
        return;

    list_move(&windows, &view->windows, &view->windowfocus->item);

    if (view->windows.tail)
        view->windowfocus = focuswindow(view->windowfocus, view->windows.tail->data);
    else
        view->windowfocus = focuswindow(view->windowfocus, 0);

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

        if (box_isinside(&view->panel.size, x, y))
            return view;

    }

    return 0;

}

static struct window *findwindow(struct view *view, unsigned int x, unsigned int y)
{

    struct list_item *current;

    for (current = view->windows.head; current; current = current->next)
    {

        struct window *window = current->data;

        if (box_isinside(&window->size, x, y))
            return window;

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

                draw(settings, screen, 1);

                break;

            case 0x10:
                if (viewfocus->windowfocus)
                {

                    send_wmquit(viewfocus->windowfocus->source);
                    unmapwindow(viewfocus);
                    arrangewindows(viewfocus);
                    draw(settings, &viewfocus->body, 1);

                }

                break;

            case 0x11:
                box_setsize(&oldmouse, mouse.image.size.x, mouse.image.size.y, mouse.image.size.w, mouse.image.size.h);

                mouse.image.size.y -= 4;

                if (mouse.image.size.y >= screen->h)
                    mouse.image.size.y = 0;

                draw(settings, &oldmouse, 1);
                draw(settings, &mouse.image.size, 1);

                break;

            case 0x19:
                spawn();

                break;

            case 0x1E:
                box_setsize(&oldmouse, mouse.image.size.x, mouse.image.size.y, mouse.image.size.w, mouse.image.size.h);

                mouse.image.size.x -= 4;

                if (mouse.image.size.x >= screen->w)
                    mouse.image.size.x = 0;

                draw(settings, &oldmouse, 1);
                draw(settings, &mouse.image.size, 1);

                break;

            case 0x1F:
                box_setsize(&oldmouse, mouse.image.size.x, mouse.image.size.y, mouse.image.size.w, mouse.image.size.h);

                mouse.image.size.y += 4;

                if (mouse.image.size.y + mouse.image.size.h > screen->h)
                    mouse.image.size.y = screen->h - mouse.image.size.h;

                draw(settings, &oldmouse, 1);
                draw(settings, &mouse.image.size, 1);

                break;

            case 0x20:
                box_setsize(&oldmouse, mouse.image.size.x, mouse.image.size.y, mouse.image.size.w, mouse.image.size.h);

                mouse.image.size.x += 4;

                if (mouse.image.size.x + mouse.image.size.w > screen->w)
                    mouse.image.size.x = screen->w - mouse.image.size.w;

                draw(settings, &oldmouse, 1);
                draw(settings, &mouse.image.size, 1);

                break;

            case 0x23:
                viewfocus->center -= (viewfocus->body.w / 32);

                arrangewindows(viewfocus);
                draw(settings, &viewfocus->body, 1);

                break;

            case 0x24:
                nextwindow(viewfocus);
                draw(settings, &viewfocus->body, 0);

                break;

            case 0x25:
                prevwindow(viewfocus);
                draw(settings, &viewfocus->body, 0);

                break;

            case 0x26:
                viewfocus->center += (viewfocus->body.w / 32);

                arrangewindows(viewfocus);
                draw(settings, &viewfocus->body, 1);

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

                    struct view *view = findview(&views, mouse.image.size.x, mouse.image.size.y);
                    struct window *window = findwindow(viewfocus, mouse.image.size.x, mouse.image.size.y);

                    if (view && view != viewfocus)
                    {

                        viewfocus = focusview(viewfocus, view);

                        draw(settings, screen, 1);

                    }

                    if (window && window != viewfocus->windowfocus)
                    {

                        viewfocus->windowfocus = focuswindow(viewfocus->windowfocus, window);

                        draw(settings, &viewfocus->body, 0);

                    }

                    break;

                }

            }

            break;

        case EVENT_MOUSEMOVE:
            box_setsize(&oldmouse, mouse.image.size.x, mouse.image.size.y, mouse.image.size.w, mouse.image.size.h);

            mouse.image.size.x += event.mousemove.relx;
            mouse.image.size.y -= event.mousemove.rely;

            if (event.mousemove.relx > 0 && mouse.image.size.x + mouse.image.size.w > screen->w)
                mouse.image.size.x = screen->w - mouse.image.size.w;

            if (event.mousemove.relx < 0 && mouse.image.size.x >= screen->w)
                mouse.image.size.x = 0;

            if (event.mousemove.rely < 0 && mouse.image.size.y + mouse.image.size.h > screen->h)
                mouse.image.size.y = screen->h - mouse.image.size.h;

            if (event.mousemove.rely > 0 && mouse.image.size.y >= screen->h)
                mouse.image.size.y = 0;

            draw(settings, &oldmouse, 1);
            draw(settings, &mouse.image.size, 1);

            break;

        case EVENT_WMMAP:
            mapwindow(viewfocus, event.header.source);
            arrangewindows(viewfocus);
            draw(settings, &viewfocus->body, 1);

            break;

        }

        if (quit)
            break;

    }

    call_close(CALL_L1);

}

static void setupwindows(void)
{

    unsigned int i;

    list_init(&windows);

    for (i = 0; i < WINDOWS; i++)
    {

        window_init(&window[i]);
        list_add(&windows, &window[i].item);

    }

}

static void setupviews(struct box *screen, struct box *menu)
{

    unsigned int i;

    list_init(&views);

    for (i = 0; i < VIEWS; i++)
    {

        view_init(&view[i], screen, menu, i, VIEWS);
        list_add(&views, &view[i].item);

    }

}

void main(void)
{

    struct ctrl_videosettings oldsettings;
    struct ctrl_videosettings settings;
    struct box screen;
    struct box menu;

    ctrl_setvideosettings(&settings, 1024, 768, 32);
    video_getmode(&oldsettings);
    video_setmode(&settings);
    draw_init();
    box_setsize(&screen, 0, 0, settings.w, settings.h);
    box_setsize(&menu, screen.x, screen.y, screen.w, 32);
    mouse_init(&mouse, &screen);
    setupwindows();
    setupviews(&screen, &menu);

    viewfocus = views.head->data;

    view_activate(viewfocus);
    draw(&settings, &screen, 0);
    pollevent(&settings, &screen);
    video_setmode(&oldsettings);

}

