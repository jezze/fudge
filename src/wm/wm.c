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

static struct ctrl_videosettings settings;
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

static void flush(unsigned int line, unsigned int offset, unsigned int count)
{

    unsigned int bpp = settings.bpp / 8;

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

void fill(unsigned int color, unsigned int offset, unsigned int count)
{

    switch (settings.bpp)
    {

    case 8:
        fill8(color, offset, count);

        break;

    case 32:
        fill32(color, offset, count);

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

static void draw(struct box *bb)
{

    unsigned int line;

    video_open();

    for (line = bb->y; line < bb->y + bb->h; line++)
    {

        drawdesktop(line);
        panel_draw(&title, line);
        drawviews(&views, line);
        mouse_draw(&mouse, line);
        flush(screen.w * line, bb->x, bb->w);

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

static unsigned int event_next(void *buffer)
{

    struct event_header *header = buffer;

    return header->count + sizeof (struct event_header);

}

static void pollevent()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff, quit = 0;
    struct view *viewactive = views.head->data;
    struct box old;

    activateview(viewactive);
    box_setsize(&old, mouse.size.x, mouse.size.y, mouse.size.w, mouse.size.h);
    draw(&screen);

    call_walk(CALL_L1, CALL_PR, 17, "system/event/poll");
    call_open(CALL_L1);

    for (roff = 0; (count = call_read(CALL_L1, roff, 1, FUDGE_BSIZE, buffer)); roff += count)
    {

        unsigned int i;

        for (i = 0; i < count; i += event_next(buffer + i))
        {

            unsigned char *temp = buffer + i;
            struct event_header *header = (struct event_header *)temp;
            unsigned char *data = temp + sizeof (struct event_header);

            switch (header->type)
            {

            case EVENT_KEYBOARD:
                if (data[0] >= 0x02 && data[0] < 0x0A)
                {

                    deactivateview(viewactive);

                    viewactive = &view[data[0] - 0x02];

                    activateview(viewactive);
                    arrangewindows(viewactive);
                    draw(&screen);

                }

                if (data[0] == 0x10)
                {

                    unmapwindow(viewactive);
                    arrangewindows(viewactive);
                    draw(&desktop);

                }

                if (data[0] == 0x19)
                {

                    spawn();

                }

                if (data[0] == 0x23)
                {

                    viewactive->center -= (desktop.w / 32);

                    arrangewindows(viewactive);
                    draw(&desktop);

                }

                if (data[0] == 0x24)
                {

                    nextwindow(viewactive);
                    draw(&desktop);

                }

                if (data[0] == 0x25)
                {

                    prevwindow(viewactive);
                    draw(&desktop);

                }

                if (data[0] == 0x26)
                {

                    viewactive->center += (desktop.w / 32);

                    arrangewindows(viewactive);
                    draw(&desktop);

                }

                if (data[0] == 0x2C)
                {

                    quit = 1;

                }

                break;

            case EVENT_MOUSE:
                {

                    mouse_handle(&mouse, data[0]);

                    if (mouse.num == 0)
                    {

                        mouse.size.x += mouse.relx;

                        if (mouse.relx < 0)
                        {

                            if (mouse.size.x >= screen.w - mouse.size.w)
                                mouse.size.x = 0;

                        }

                        else
                        {

                            if (mouse.size.x >= screen.w - mouse.size.w)
                                mouse.size.x = screen.w - mouse.size.w;

                        }

                        mouse.size.y -= mouse.rely;

                        if (mouse.rely > 0)
                        {

                            if (mouse.size.y >= screen.h - mouse.size.h)
                                mouse.size.y = 0;

                        }

                        else
                        {

                            if (mouse.size.y >= screen.h - mouse.size.h)
                                mouse.size.y = screen.h - mouse.size.h;

                        }

                        draw(&mouse.size);
                        draw(&old);

                        old.x = mouse.size.x;
                        old.y = mouse.size.y;

                    }

                }

                break;

            case 1000:
                mapwindow(viewactive, header->source);
                arrangewindows(viewactive);
                draw(&desktop);

                break;

            }


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

    settings.header.type = CTRL_TYPE_VIDEO;
    settings.w = 1024;
    settings.h = 768;
    settings.bpp = 32;

    video_setmode(&settings);
    video_setcolormap(0, 27, colormap);
    mouse_init(&mouse);
    box_setsize(&screen, 0, 0, settings.w, settings.h);
    box_setsize(&menu, screen.x, screen.y, screen.w, BOXSIZE);
    box_setsize(&desktop, screen.x, screen.y + BOXSIZE, screen.w, screen.h - BOXSIZE);
    box_setsize(&mouse.size, screen.x + screen.w / 4, screen.y + screen.h / 4, mouse.size.w, mouse.size.h);
    panel_init(&title);
    box_setsize(&title.size, menu.x + VIEWS * BOXSIZE, menu.y, menu.w - VIEWS * BOXSIZE, BOXSIZE);
    setupwindows();
    setupviews();
    pollevent();

    settings.header.type = CTRL_TYPE_VIDEO;
    settings.w = 80;
    settings.h = 25;
    settings.bpp = 16;

    video_setmode(&settings);

}

