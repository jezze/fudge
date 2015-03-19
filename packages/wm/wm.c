#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "draw.h"
#include "text.h"
#include "panel.h"
#include "window.h"
#include "view.h"

#define WINDOWS                         64
#define VIEWS                           8

struct event_header
{

    unsigned int destination;
    unsigned int source;
    unsigned int type;
    unsigned int count;

};

static struct box screen;
static struct box menu;
static struct box desktop;
static struct box mouse;
static struct window window[WINDOWS];
static struct list windows;
static struct view view[VIEWS];
static struct list views;
static struct view *viewactive;

static void drawwindows(struct list *windows, unsigned int line)
{

    struct list_item *current;

    for (current = windows->head; current; current = current->next)
    {

        struct window *window = current->data;

        window_draw(window, line);

    }

}

static void drawviews(struct list *views, unsigned int line)
{

    struct list_item *current;

    for (current = views->head; current; current = current->next)
    {

        struct view * view = current->data;

        panel_draw(&view->panel, line);

        if (!view->active)
            continue;

        drawwindows(&view->windows, line);

    }

}

static void draw(unsigned int start, unsigned int stop)
{

    unsigned int i;

    draw_begin();

    for (i = start; i < stop; i++)
    {

        backbuffer_fillbox(&menu, WM_COLOR_DARK, i);
        backbuffer_fillbox(&desktop, WM_COLOR_BODY, i);
        drawviews(&views, i);
        backbuffer_fillbox(&mouse, WM_COLOR_LIGHT, i);
        backbuffer_drawline(i);

    }

    draw_end();

}

static void spawn()
{

    call_walk(CALL_CP, CALL_PR, 9, "bin/wnull");
    call_spawn();

}

static void arrangewindows(struct view *view)
{

    unsigned int count = list_count(&view->windows);
    struct list_item *current = view->windows.head;
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

    box_setsize(&window->size, desktop.x, desktop.y, desktop.w / 2, desktop.h);

    a = desktop.h / (count - 1);
    i = 0;

    for (current = current->next; current; current = current->next)
    {

        window = current->data;

        box_setsize(&window->size, desktop.x + desktop.w / 2, desktop.y + i * a, desktop.w / 2, a);

        i++;

    }

}

static void activatewindow(struct view *view, struct window *window)
{

    if (!view->windowactive)
        view->windowactive = window;

    view->windowactive->active = 0;
    view->windowactive = window;
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

    draw(0, SCREEN_HEIGHT);

}

static void prevwindow(struct view *view)
{

    if (!view->windowactive)
        return;

    if (view->windowactive->item.prev)
        activatewindow(view, view->windowactive->item.prev->data);
    else
        activatewindow(view, view->windows.tail->data);

    draw(0, SCREEN_HEIGHT);

}

static void mapwindow(struct view *view)
{

    struct window *window;

    if (!windows.head)
        return;

    window = windows.head->data;

    list_move(&view->windows, &windows, &window->item);
    arrangewindows(view);
    activatewindow(view, window);
    draw(0, SCREEN_HEIGHT);

}

static void activateview(struct view *v)
{

    if (viewactive == v)
        return;

    viewactive->active = 0;
    viewactive->panel.active = 0;
    viewactive = v;
    viewactive->active = 1;
    viewactive->panel.active = 1;

    draw(0, SCREEN_HEIGHT);

}

static unsigned int mp = 0;
static unsigned char state;
static unsigned int oldx;
static unsigned int oldy;

static void mouseinput(unsigned char value)
{

    char relx;
    char rely;

    switch (mp)
    {

    case 0:
        state = value;
        mp = 1;

        break;

    case 1:
        relx = value - ((state << 4) & 0x100);
        oldx = mouse.x;
        mouse.x += relx / 2;
        mp = 2;

        break;

    case 2:
        rely = value - ((state << 3) & 0x100);
        oldy = mouse.y;
        mouse.y -= rely / 2;
        mp = 0;

        break;

    }

    draw(oldy, oldy + mouse.h);
    draw(mouse.y, mouse.y + mouse.h);

}

static unsigned int event_next(void *buffer)
{

    struct event_header *header = buffer;

    return header->count + sizeof (struct event_header);

}

static void sendevent(unsigned int type)
{

    struct event_header header;

    header.type = type;
    header.count = 0;

    call_walk(CALL_L2, CALL_PR, 17, "system/event/send");
    call_open(CALL_L2);
    call_write(CALL_L2, 0, sizeof (struct event_header), &header);
    call_close(CALL_L2);

}

static void pollevent()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff;

    call_walk(CALL_L1, CALL_PR, 15, "system/event/wm");
    call_open(CALL_L1);

    for (roff = 0; (count = call_read(CALL_L1, roff, FUDGE_BSIZE, buffer)); roff += count)
    {

        unsigned int i;

        for (i = 0; i < count; i += event_next(buffer + i))
        {

            unsigned char *temp = buffer + i;
            struct event_header *header = (struct event_header *)temp;
            unsigned char *data = temp + sizeof (struct event_header);

            switch (header->type)
            {

            case 1:
                if (data[0] == 0x02)
                    activateview(&view[0]);

                if (data[0] == 0x03)
                    activateview(&view[1]);

                if (data[0] == 0x04)
                    activateview(&view[2]);

                if (data[0] == 0x05)
                    activateview(&view[3]);

                if (data[0] == 0x06)
                    activateview(&view[4]);

                if (data[0] == 0x07)
                    activateview(&view[5]);

                if (data[0] == 0x08)
                    activateview(&view[6]);

                if (data[0] == 0x09)
                    activateview(&view[7]);

                if (data[0] == 0x10)
                    sendevent(1001);

                if (data[0] == 0x19)
                    spawn();

                if (data[0] == 0x24)
                    nextwindow(viewactive);

                if (data[0] == 0x25)
                    prevwindow(viewactive);

                break;

            case 2:
                mouseinput(data[0]);

                break;

            case 1000:
                mapwindow(viewactive);

                break;

            }


        }

    }

    call_close(CALL_L1);

}

void setupwindows()
{

    unsigned int i;

    list_init(&windows);

    for (i = 0; i < WINDOWS; i++)
    {

        window_init(&window[i], "0", 0);
        list_add(&windows, &window[i].item);

    }

}

void setupviews()
{

    unsigned int i;

    list_init(&views);

    for (i = 0; i < VIEWS; i++)
    {

        view_init(&view[i], "0", 0);
        box_setsize(&view[i].panel.size, menu.x + i * BOXSIZE, menu.y, BOXSIZE, BOXSIZE);
        list_add(&views, &view[i].item);

    }

    viewactive = views.head->data;
    viewactive->active = 1;
    viewactive->panel.active = 1;

}

void main()
{

    box_setsize(&screen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    box_setsize(&menu, screen.x, screen.y, screen.w, BOXSIZE);
    box_setsize(&desktop, screen.x, screen.y + BOXSIZE, screen.w, screen.h - BOXSIZE);
    box_setsize(&mouse, screen.x + SCREEN_WIDTH / 4, screen.y + SCREEN_HEIGHT / 4, 16, 16);
    setupwindows();
    setupviews();
    draw_setmode();
    draw_setcolormap();
    draw_begin();
    draw_end();
    draw(0, SCREEN_HEIGHT);
    pollevent();

}

