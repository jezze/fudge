#include <abi.h>
#include <fudge.h>
#include <video/video.h>
#include "box.h"
#include "draw.h"
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

static unsigned char colormap[] = {
    0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF,
    0x03, 0x02, 0x02,
    0x05, 0x04, 0x04,
    0x07, 0x06, 0x06,
    0x08, 0x10, 0x18,
    0x0C, 0x14, 0x1C,
    0x28, 0x10, 0x18,
    0x38, 0x20, 0x28
};

static unsigned int colormap4[256] = {
    0xFF000000,
    0xFFFFFFFF,
    0xFF181014,
    0xFF20181C,
    0xFF30282C,
    0xFF105070,
    0xFF307090,
    0xFFB05070,
    0xFFF898B8
};

static void spawn()
{

    call_walk(CALL_CP, CALL_PR, 9, "bin/wnull");
    call_spawn();

}

void fill(unsigned int bpp, unsigned int color, unsigned int offset, unsigned int count)
{

    switch (bpp)
    {

    case 8:
        fill8(color, offset, count);

        break;

    case 32:
        fill32(colormap4[color], offset, count);

        break;

    }

}

static void drawviews(struct list *views, unsigned int bpp, unsigned int line)
{

    struct list_item *current;

    for (current = views->head; current; current = current->next)
    {

        struct view *view = current->data;

        view_draw(view, bpp, line);

    }

}

static void sendwmresizeall(struct view *view)
{

    struct list_item *current;

    for (current = view->windows.head; current; current = current->next)
    {

        struct window *window = current->data;

        send_wmresize(window->source, window->size.x + 3, window->size.y + 3, window->size.w - 6, window->size.h - 6);

    }

}

static void sendwmdrawall(struct view *view, struct box *bb)
{

    struct list_item *current;

    for (current = view->windows.head; current; current = current->next)
    {

        struct window *window = current->data;

        send_wmdraw(window->source, bb->x, bb->y, bb->w, bb->h);

    }

}

static void draw(struct ctrl_videosettings *settings, struct box *bb, struct view *view, unsigned int notify)
{

    unsigned int line;

    video_open();

    for (line = bb->y; line < bb->y + bb->h; line++)
    {

        fill(settings->bpp, WM_COLOR_TRANSPARENT, bb->x, bb->w);
        drawviews(&views, settings->bpp, line);
        flush(settings->w * line, settings->bpp, bb->x, bb->w);

    }

    video_close();

    if (notify)
        sendwmdrawall(view, bb);

    video_open();

    for (line = bb->y; line < bb->y + bb->h; line++)
    {

        fill(settings->bpp, WM_COLOR_TRANSPARENT, bb->x, bb->w);
        mouse_draw(&mouse, settings->bpp, line);
        flush(settings->w * line, settings->bpp, bb->x, bb->w);

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

        return;

    }

    box_setsize(&window->size, view->body.x, view->body.y, view->center, view->body.h);

    a = view->body.h / (count - 1);
    i = 0;

    for (current = current->prev; current; current = current->prev)
    {

        window = current->data;

        box_setsize(&window->size, view->body.x + view->center, view->body.y + i * a, view->body.w - view->center, a);

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

static struct view *findview(struct list *views, struct mouse *mouse)
{

    struct list_item *current;

    for (current = views->head; current; current = current->next)
    {

        struct view *view = current->data;

        if (box_isinside(&view->panel.size, mouse->size.x, mouse->size.y))
            return view;

    }

    return 0;

}

static struct window *findwindow(struct view *view, struct mouse *mouse)
{

    struct list_item *current;

    for (current = view->windows.head; current; current = current->next)
    {

        struct window *window = current->data;

        if (box_isinside(&window->size, mouse->size.x, mouse->size.y))
            return window;

    }

    return 0;

}

static void pollevent(struct ctrl_videosettings *settings, struct box *screen)
{

    union event event;
    unsigned int count, roff, quit = 0;
    struct view *viewfocus = views.head->data;
    struct box oldmouse;

    view_activate(viewfocus);
    draw(settings, screen, viewfocus, 0);

    call_walk(CALL_L1, CALL_PR, 17, "system/event/poll");
    call_open(CALL_L1);

    for (roff = 0; (count = call_read(CALL_L1, roff, sizeof (struct event_header), 1, &event.header)); roff += count)
    {

        box_setsize(&oldmouse, mouse.size.x, mouse.size.y, mouse.size.w, mouse.size.h);

        if (event.header.count)
            count += call_read(CALL_L1, roff + count, event.header.count, 1, event.data + sizeof (struct event_header));

        switch (event.header.type)
        {

        case EVENT_KEYPRESS:
            if (event.keypress.scancode >= 0x02 && event.keypress.scancode < 0x0A)
            {

                viewfocus = focusview(viewfocus, &view[event.keypress.scancode - 0x02]);

                arrangewindows(viewfocus);
                draw(settings, screen, viewfocus, 1);

            }

            if (event.keypress.scancode == 0x10)
            {

                if (viewfocus->windowfocus)
                {

                    send_wmquit(viewfocus->windowfocus->source);
                    unmapwindow(viewfocus);
                    arrangewindows(viewfocus);
                    sendwmresizeall(viewfocus);
                    draw(settings, &viewfocus->body, viewfocus, 1);

                }

            }

            if (event.keypress.scancode == 0x11)
            {

                mouse.size.y -= 4;

                if (mouse.size.y >= screen->h)
                    mouse.size.y = 0;

                draw(settings, &oldmouse, viewfocus, 1);
                draw(settings, &mouse.size, viewfocus, 1);

            }

            if (event.keypress.scancode == 0x19)
            {

                spawn();

            }

            if (event.keypress.scancode == 0x1F)
            {

                mouse.size.y += 4;

                if (mouse.size.y + mouse.size.h > screen->h)
                    mouse.size.y = screen->h - mouse.size.h;

                draw(settings, &oldmouse, viewfocus, 1);
                draw(settings, &mouse.size, viewfocus, 1);

            }

            if (event.keypress.scancode == 0x1E)
            {

                mouse.size.x -= 4;

                if (mouse.size.x >= screen->w)
                    mouse.size.x = 0;

                draw(settings, &oldmouse, viewfocus, 1);
                draw(settings, &mouse.size, viewfocus, 1);

            }

            if (event.keypress.scancode == 0x20)
            {

                mouse.size.x += 4;

                if (mouse.size.x + mouse.size.w > screen->w)
                    mouse.size.x = screen->w - mouse.size.w;

                draw(settings, &oldmouse, viewfocus, 1);
                draw(settings, &mouse.size, viewfocus, 1);

            }

            if (event.keypress.scancode == 0x23)
            {

                viewfocus->center -= (viewfocus->body.w / 32);

                arrangewindows(viewfocus);
                sendwmresizeall(viewfocus);
                draw(settings, &viewfocus->body, viewfocus, 1);

            }

            if (event.keypress.scancode == 0x24)
            {

                nextwindow(viewfocus);
                draw(settings, &viewfocus->body, viewfocus, 0);

            }

            if (event.keypress.scancode == 0x25)
            {

                prevwindow(viewfocus);
                draw(settings, &viewfocus->body, viewfocus, 0);

            }

            if (event.keypress.scancode == 0x26)
            {

                viewfocus->center += (viewfocus->body.w / 32);

                arrangewindows(viewfocus);
                sendwmresizeall(viewfocus);
                draw(settings, &viewfocus->body, viewfocus, 1);

            }

            if (event.keypress.scancode == 0x2C)
            {

                quit = 1;

            }

            break;

        case EVENT_MOUSEPRESS:
            if (event.mousepress.button == 1)
            {

                struct view *view = findview(&views, &mouse);
                struct window *window = findwindow(viewfocus, &mouse);

                if (view && view != viewfocus)
                {

                    viewfocus = focusview(viewfocus, view);

                    arrangewindows(viewfocus);
                    draw(settings, screen, viewfocus, 1);

                }

                if (window && window != viewfocus->windowfocus)
                {

                    viewfocus->windowfocus = focuswindow(viewfocus->windowfocus, window);

                    draw(settings, &viewfocus->body, viewfocus, 0);

                }

            }

            break;

        case EVENT_MOUSEMOVE:
            mouse.size.x += event.mousemove.relx;
            mouse.size.y -= event.mousemove.rely;

            if (event.mousemove.relx > 0 && mouse.size.x + mouse.size.w > screen->w)
                mouse.size.x = screen->w - mouse.size.w;

            if (event.mousemove.relx < 0 && mouse.size.x >= screen->w)
                mouse.size.x = 0;

            if (event.mousemove.rely < 0 && mouse.size.y + mouse.size.h > screen->h)
                mouse.size.y = screen->h - mouse.size.h;

            if (event.mousemove.rely > 0 && mouse.size.y >= screen->h)
                mouse.size.y = 0;

            draw(settings, &oldmouse, viewfocus, 1);
            draw(settings, &mouse.size, viewfocus, 1);

            break;

        case EVENT_WMMAP:
            mapwindow(viewfocus, event.header.source);
            arrangewindows(viewfocus);
            sendwmresizeall(viewfocus);
            draw(settings, &viewfocus->body, viewfocus, 1);

            break;

        }

        if (quit)
            break;

    }

    call_close(CALL_L1);

}

static void setupwindows()
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

void main()
{

    struct ctrl_videosettings oldsettings;
    struct ctrl_videosettings settings;
    struct box screen;
    struct box menu;

    colormap4[WM_COLOR_TRANSPARENT] = 0x00FF00FF;

    ctrl_setvideosettings(&settings, 1024, 768, 32);
    video_getmode(&oldsettings);
    video_setmode(&settings);
    video_setcolormap(0, 3, 9, colormap);
    box_setsize(&screen, 0, 0, settings.w, settings.h);
    box_setsize(&menu, screen.x, screen.y, screen.w, 32);
    mouse_init(&mouse, &screen);
    setupwindows();
    setupviews(&screen, &menu);
    pollevent(&settings, &screen);
    video_setmode(&oldsettings);

}

