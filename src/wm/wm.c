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

static struct box screen;
static struct box menu;
static struct box desktop;
static struct mouse mouse;
static struct panel title;
static struct window window[WINDOWS];
static struct list windows;
static struct view view[VIEWS];
static struct list views;
static struct ctrl_videosettings oldsettings;
static struct ctrl_videosettings settings;

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

static unsigned int colormap4[] = {
    0x00000000,
    0x00FFFFFF,
    0x00181014,
    0x0020181C,
    0x0030282C,
    0x00105070,
    0x00307090,
    0x00B05070,
    0x00F898B8
};

static void spawn()
{

    call_walk(CALL_CP, CALL_PR, 9, "bin/wnull");
    call_spawn();

}

void fill(unsigned int color, unsigned int offset, unsigned int count)
{

    switch (settings.bpp)
    {

    case 8:
        fill8(color, offset, count);

        break;

    case 32:
        fill32(colormap4[color], offset, count);

        break;

    }

}

static void drawdesktop(unsigned int line)
{

    if (line < desktop.y || line >= desktop.y + desktop.h)
        return;

    fill(WM_COLOR_BODY, desktop.x, desktop.w);

}

static void drawviews(struct list *views, unsigned int line)
{

    struct list_item *current;

    for (current = views->head; current; current = current->next)
    {

        struct view *view = current->data;

        view_draw(view, line);

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

static void draw(struct view *view, struct box *bb)
{

    unsigned int line;

    video_open();

    for (line = bb->y; line < bb->y + bb->h; line++)
    {

        if (!view->windowactive)
            drawdesktop(line);

        panel_draw(&title, line);
        drawviews(&views, line);
        mouse_draw(&mouse, line);
        flush(settings.w * line, settings.bpp, bb->x, bb->w);

    }

    video_close();
    sendwmdrawall(view, bb);

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

        box_setsize(&window->size, desktop.x, desktop.y, desktop.w, desktop.h);

        return;

    }

    box_setsize(&window->size, desktop.x, desktop.y, view->center, desktop.h);

    a = desktop.h / (count - 1);
    i = 0;

    for (current = current->prev; current; current = current->prev)
    {

        window = current->data;

        box_setsize(&window->size, desktop.x + view->center, desktop.y + i * a, desktop.w - view->center, a);

        i++;

    }

}

static void activatewindow(struct view *view, struct window *window)
{

    if (view->windowactive)
        view->windowactive->active = 0;

    view->windowactive = window;

    if (view->windowactive)
        view->windowactive->active = 1;

}

static void nextwindow(struct view *view)
{

    if (!view->windowactive)
        return;

    if (view->windowactive->item.next)
        activatewindow(view, view->windowactive->item.next->data);
    else
        activatewindow(view, view->windows.head->data);

}

static void prevwindow(struct view *view)
{

    if (!view->windowactive)
        return;

    if (view->windowactive->item.prev)
        activatewindow(view, view->windowactive->item.prev->data);
    else
        activatewindow(view, view->windows.tail->data);

}

static void mapwindow(struct view *view, unsigned int source)
{

    struct window *window;

    if (!windows.head)
        return;

    window = windows.head->data;
    window->source = source;

    list_move(&view->windows, &windows, &window->item);
    activatewindow(view, window);

}

static void unmapwindow(struct view *view)
{

    if (!view->windowactive)
        return;

    list_move(&windows, &view->windows, &view->windowactive->item);

    if (view->windows.tail)
        activatewindow(view, view->windows.tail->data);
    else
        activatewindow(view, 0);

}

static void activateview(struct view *view)
{

    view->active = 1;
    view->panel.active = 1;

}

static void deactivateview(struct view *view)
{

    view->active = 0;
    view->panel.active = 0;

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

static void pollevent()
{

    union event event;
    unsigned int count, roff, quit = 0;
    struct view *viewactive = views.head->data;
    struct box old;

    activateview(viewactive);
    box_setsize(&old, mouse.size.x, mouse.size.y, mouse.size.w, mouse.size.h);
    draw(viewactive, &screen);

    call_walk(CALL_L1, CALL_PR, 17, "system/event/poll");
    call_open(CALL_L1);

    for (roff = 0; (count = call_read(CALL_L1, roff, sizeof (struct event_header), 1, &event.header)); roff += count)
    {

        if (event.header.count)
            count += call_read(CALL_L1, roff + count, event.header.count, 1, event.data + sizeof (struct event_header));

        switch (event.header.type)
        {

        case EVENT_KEYPRESS:
            if (event.keypress.scancode >= 0x02 && event.keypress.scancode < 0x0A)
            {

                deactivateview(viewactive);

                viewactive = &view[event.keypress.scancode - 0x02];

                activateview(viewactive);
                arrangewindows(viewactive);
                draw(viewactive, &screen);

            }

            if (event.keypress.scancode == 0x10)
            {

                if (viewactive->windowactive)
                {

                    send_wmquit(viewactive->windowactive->source);
                    unmapwindow(viewactive);
                    arrangewindows(viewactive);
                    sendwmresizeall(viewactive);
                    draw(viewactive, &desktop);

                }

            }

            if (event.keypress.scancode == 0x11)
            {

                old.x = mouse.size.x;
                old.y = mouse.size.y;
                mouse.size.y -= 4;

                if (mouse.size.y >= settings.h)
                    mouse.size.y = 0;

                draw(viewactive, &old);
                draw(viewactive, &mouse.size);

            }

            if (event.keypress.scancode == 0x19)
            {

                spawn();

            }

            if (event.keypress.scancode == 0x1F)
            {

                old.x = mouse.size.x;
                old.y = mouse.size.y;
                mouse.size.y += 4;

                if (mouse.size.y + mouse.size.h > settings.h)
                    mouse.size.y = settings.h - mouse.size.h;

                draw(viewactive, &old);
                draw(viewactive, &mouse.size);

            }

            if (event.keypress.scancode == 0x1E)
            {

                old.x = mouse.size.x;
                old.y = mouse.size.y;
                mouse.size.x -= 4;

                if (mouse.size.x >= settings.w)
                    mouse.size.x = 0;

                draw(viewactive, &old);
                draw(viewactive, &mouse.size);

            }

            if (event.keypress.scancode == 0x20)
            {

                old.x = mouse.size.x;
                old.y = mouse.size.y;
                mouse.size.x += 4;

                if (mouse.size.x + mouse.size.w > settings.w)
                    mouse.size.x = settings.w - mouse.size.w;

                draw(viewactive, &old);
                draw(viewactive, &mouse.size);

            }

            if (event.keypress.scancode == 0x23)
            {

                viewactive->center -= (desktop.w / 32);

                arrangewindows(viewactive);
                sendwmresizeall(viewactive);
                draw(viewactive, &desktop);

            }

            if (event.keypress.scancode == 0x24)
            {

                nextwindow(viewactive);
                draw(viewactive, &desktop);

            }

            if (event.keypress.scancode == 0x25)
            {

                prevwindow(viewactive);
                draw(viewactive, &desktop);

            }

            if (event.keypress.scancode == 0x26)
            {

                viewactive->center += (desktop.w / 32);

                arrangewindows(viewactive);
                sendwmresizeall(viewactive);
                draw(viewactive, &desktop);

            }

            if (event.keypress.scancode == 0x2C)
            {

                quit = 1;

            }

            break;

        case EVENT_MOUSEPRESS:
            if (event.mousepress.button == 1)
            {

                struct view *view;
                struct window *window;
                
                view = findview(&views, &mouse);

                if (view && view != viewactive)
                {

                    deactivateview(viewactive);

                    viewactive = view;

                    activateview(viewactive);
                    arrangewindows(viewactive);
                    draw(viewactive, &screen);

                }

                window = findwindow(viewactive, &mouse);

                if (window && window != viewactive->windowactive)
                {

                    activatewindow(viewactive, window);
                    draw(viewactive, &desktop);

                }

            }

            break;

        case EVENT_MOUSEMOVE:
            old.x = mouse.size.x;
            old.y = mouse.size.y;
            mouse.size.x += event.mousemove.relx;
            mouse.size.y -= event.mousemove.rely;

            if (event.mousemove.relx > 0 && mouse.size.x + mouse.size.w > settings.w)
                mouse.size.x = settings.w - mouse.size.w;

            if (event.mousemove.relx < 0 && mouse.size.x >= settings.w)
                mouse.size.x = 0;

            if (event.mousemove.rely < 0 && mouse.size.y + mouse.size.h > settings.h)
                mouse.size.y = settings.h - mouse.size.h;

            if (event.mousemove.rely > 0 && mouse.size.y >= settings.h)
                mouse.size.y = 0;

            draw(viewactive, &old);
            draw(viewactive, &mouse.size);

            break;

        case EVENT_WMMAP:
            mapwindow(viewactive, event.header.source);
            arrangewindows(viewactive);
            sendwmresizeall(viewactive);
            draw(viewactive, &desktop);

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

static void setupviews()
{

    unsigned int i;

    list_init(&views);

    for (i = 0; i < VIEWS; i++)
    {

        view_init(&view[i], desktop.w / 2);
        box_setsize(&view[i].panel.size, menu.x + i * BOXSIZE, menu.y, BOXSIZE, BOXSIZE);
        list_add(&views, &view[i].item);

    }

}

void main()
{

    ctrl_setvideosettings(&settings, 1024, 768, 32);
    video_getmode(&oldsettings);
    video_setmode(&settings);
    video_setcolormap(0, 3, 9, colormap);
    mouse_init(&mouse);
    box_setsize(&screen, 0, 0, settings.w, settings.h);
    box_setsize(&menu, screen.x, screen.y, screen.w, BOXSIZE);
    box_setsize(&desktop, screen.x, screen.y + BOXSIZE, screen.w, screen.h - BOXSIZE);
    box_setsize(&mouse.size, screen.x + screen.w / 4, screen.y + screen.h / 4, mouse.size.w, mouse.size.h);
    panel_init(&title);
    box_setsize(&title.size, menu.x + VIEWS * BOXSIZE, menu.y, menu.w - VIEWS * BOXSIZE, BOXSIZE);
    setupwindows();
    setupviews();
    pollevent(&settings);
    video_setmode(&oldsettings);

}

