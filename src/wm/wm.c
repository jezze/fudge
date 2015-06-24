#include <abi.h>
#include <fudge.h>
#include <video/video.h>
#include "box.h"
#include "draw.h"
#include "mouse.h"
#include "panel.h"
#include "window.h"
#include "view.h"

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
static unsigned char backbuffer[4096];

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
    0x000000,
    0xFFFFFF,
    0x181014,
    0x20181C,
    0x30282C,
    0x105070,
    0x307090,
    0xB05070,
    0xF898B8
};

static void flush(struct ctrl_videosettings *settings, unsigned int line, unsigned int offset, unsigned int count)
{

    unsigned int bpp = settings->bpp / 8;

    video_draw(line * bpp + offset * bpp, count * bpp, backbuffer + offset * bpp);

}

static void fill8(unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned char *b = (unsigned char *)backbuffer;
    unsigned int i;

    for (i = offset; i < count + offset; i++)
        b[i] = color;

}

static void fill32(unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned int *b = (unsigned int *)backbuffer;
    unsigned int i;

    for (i = offset; i < count + offset; i++)
        b[i] = colormap4[color];

}

void fill(struct ctrl_videosettings *settings, unsigned int color, unsigned int offset, unsigned int count)
{

    switch (settings->bpp)
    {

    case 8:
        fill8(color, offset, count);

        break;

    case 32:
        fill32(color, offset, count);

        break;

    }

}

static void drawdesktop(struct ctrl_videosettings *settings, unsigned int line)
{

    if (line < desktop.y || line >= desktop.y + desktop.h)
        return;

    fill(settings, WM_COLOR_BODY, desktop.x, desktop.w);

}

static void drawviews(struct list *views, struct ctrl_videosettings *settings, unsigned int line)
{

    struct list_item *current;

    for (current = views->head; current; current = current->next)
    {

        struct view *view = current->data;

        view_draw(view, settings, line);

    }

}

static void draw(struct ctrl_videosettings *settings, struct box *bb)
{

    unsigned int line;

    video_open();

    for (line = bb->y; line < bb->y + bb->h; line++)
    {

        drawdesktop(settings, line);
        panel_draw(&title, settings, line);
        drawviews(&views, settings, line);
        mouse_draw(&mouse, settings, line);
        flush(settings, screen.w * line, bb->x, bb->w);

    }

    video_close();

}

static void spawn()
{

    call_walk(CALL_CP, CALL_PR, 9, "bin/wnull");
    call_spawn();

}

static void sendevent(unsigned int destination, unsigned int type)
{

    struct event_header header;

    header.destination = destination;
    header.type = type;
    header.count = 0;

    call_walk(CALL_L2, CALL_PR, 17, "system/event/send");
    call_open(CALL_L2);
    call_write(CALL_L2, 0, sizeof (struct event_header), 1, &header);
    call_close(CALL_L2);

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

    sendevent(view->windowactive->source, 1001);
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

static void pollevent(struct ctrl_videosettings *settings)
{

    struct event_header header;
    unsigned int count, roff, quit = 0;
    struct view *viewactive = views.head->data;
    struct box old;

    activateview(viewactive);
    box_setsize(&old, mouse.size.x, mouse.size.y, mouse.size.w, mouse.size.h);
    draw(settings, &screen);

    call_walk(CALL_L1, CALL_PR, 17, "system/event/poll");
    call_open(CALL_L1);

    for (roff = 0; (count = call_read(CALL_L1, roff, sizeof (struct event_header), 1, &header)); roff += count)
    {

        char data[32];

        if (header.count)
            count += call_read(CALL_L1, roff + count, header.count, 1, data);

        switch (header.type)
        {

        case EVENT_KEYPRESS:
            if (data[0] >= 0x02 && data[0] < 0x0A)
            {

                deactivateview(viewactive);

                viewactive = &view[data[0] - 0x02];

                activateview(viewactive);
                arrangewindows(viewactive);
                draw(settings, &screen);

            }

            if (data[0] == 0x10)
            {

                unmapwindow(viewactive);
                arrangewindows(viewactive);
                draw(settings, &desktop);

            }

            if (data[0] == 0x19)
            {

                spawn();

            }

            if (data[0] == 0x23)
            {

                viewactive->center -= (desktop.w / 32);

                arrangewindows(viewactive);
                draw(settings, &desktop);

            }

            if (data[0] == 0x24)
            {

                nextwindow(viewactive);
                draw(settings, &desktop);

            }

            if (data[0] == 0x25)
            {

                prevwindow(viewactive);
                draw(settings, &desktop);

            }

            if (data[0] == 0x26)
            {

                viewactive->center += (desktop.w / 32);

                arrangewindows(viewactive);
                draw(settings, &desktop);

            }

            if (data[0] == 0x2C)
            {

                quit = 1;

            }

            break;

        case EVENT_MOUSEPRESS:
            if (data[0] == 1)
            {

                struct view *view = findview(&views, &mouse);

                if (view)
                {

                    deactivateview(viewactive);

                    viewactive = view;

                    activateview(viewactive);
                    arrangewindows(viewactive);
                    draw(settings, &screen);

                }

            }

            break;

        case EVENT_MOUSEMOVE:
            mouse.size.x += data[0];
            mouse.size.y -= data[1];

            if (data[0] > 0 && mouse.size.x + mouse.size.w > settings->w)
                mouse.size.x = settings->w - mouse.size.w;

            if (data[0] < 0 && mouse.size.x >= settings->w)
                mouse.size.x = 0;

            if (data[1] < 0 && mouse.size.y + mouse.size.h > settings->h)
                mouse.size.y = settings->h - mouse.size.h;

            if (data[1] > 0 && mouse.size.y >= settings->h)
                mouse.size.y = 0;

            draw(settings, &mouse.size);
            draw(settings, &old);

            old.x = mouse.size.x;
            old.y = mouse.size.y;

            break;

        case EVENT_WMMAP:
            mapwindow(viewactive, header.source);
            arrangewindows(viewactive);
            draw(settings, &desktop);

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

    struct ctrl_videosettings oldsettings;
    struct ctrl_videosettings newsettings;

    ctrl_setvideosettings(&newsettings, 1024, 768, 32);
    video_getmode(&oldsettings);
    video_setmode(&newsettings);
    video_setcolormap(0, 3, 9, colormap);
    mouse_init(&mouse);
    box_setsize(&screen, 0, 0, newsettings.w, newsettings.h);
    box_setsize(&menu, screen.x, screen.y, screen.w, BOXSIZE);
    box_setsize(&desktop, screen.x, screen.y + BOXSIZE, screen.w, screen.h - BOXSIZE);
    box_setsize(&mouse.size, screen.x + screen.w / 4, screen.y + screen.h / 4, mouse.size.w, mouse.size.h);
    panel_init(&title);
    box_setsize(&title.size, menu.x + VIEWS * BOXSIZE, menu.y, menu.w - VIEWS * BOXSIZE, BOXSIZE);
    setupwindows();
    setupviews();
    pollevent(&newsettings);
    video_setmode(&oldsettings);

}

