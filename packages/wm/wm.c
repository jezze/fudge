#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "draw.h"
#include "text.h"
#include "panel.h"
#include "window.h"
#include "view.h"

#define WINDOWS                         64
#define VIEWS                           4

struct event_header
{

    unsigned int destination;
    unsigned int source;
    unsigned int type;
    unsigned int count;

};

static struct box back;
static struct box empty;
static struct panel field;
static struct panel clock;
static struct window window[WINDOWS];
static struct list windows;
static struct view view[VIEWS];
static struct list views;
static struct view *viewactive;

static void drawwindows(struct list *windows)
{

    struct list_item *current;

    for (current = windows->head; current; current = current->next)
    {

        struct window *window = current->data;

        window_draw(window);

    }

}

static void drawviews(struct list *views)
{

    struct list_item *current;

    for (current = views->head; current; current = current->next)
    {

        struct view * view = current->data;

        panel_draw(&view->panel);

        if (!view->active)
            continue;

        if (view->windows.head)
            drawwindows(&view->windows);
        else
            backbuffer_fillbox(&empty, WM_COLOR_BODY);

    }

}

static void draw()
{

    draw_begin();
    panel_draw(&field);
    panel_draw(&clock);
    drawviews(&views);
    draw_end();

}

static void spawn()
{

    call_walk(CALL_CPP, CALL_DR, 9, "bin/wnull");
    call_walk(CALL_CI0, CALL_I0, 0, 0);
    call_walk(CALL_CO0, CALL_O0, 0, 0);
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

        box_setsize(&window->size, SCREEN_BORDER, 16 + SCREEN_BORDER, SCREEN_WIDTH - 2 * SCREEN_BORDER, SCREEN_HEIGHT - 16 - 2 * SCREEN_BORDER);

        return;

    }

    box_setsize(&window->size, SCREEN_BORDER, 16 + SCREEN_BORDER, SCREEN_WIDTH / 2 - SCREEN_BORDER, SCREEN_HEIGHT - 16 - 2 * SCREEN_BORDER);

    a = (SCREEN_HEIGHT - 16 - 2 * SCREEN_BORDER) / (count - 1);
    i = 0;

    for (current = current->next; current; current = current->next)
    {

        window = current->data;

        box_setsize(&window->size, SCREEN_WIDTH / 2, (16 + SCREEN_BORDER) + i * a, SCREEN_WIDTH / 2 - SCREEN_BORDER, a);

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

    draw();

}

static void prevwindow(struct view *view)
{

    if (!view->windowactive)
        return;

    if (view->windowactive->item.prev)
        activatewindow(view, view->windowactive->item.prev->data);
    else
        activatewindow(view, view->windows.tail->data);

    draw();

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
    draw();

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

    draw();

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

    call_walk(CALL_L2, CALL_DR, 17, "system/event/send");
    call_open(CALL_L2);
    call_write(CALL_L2, 0, sizeof (struct event_header), &header);
    call_close(CALL_L2);

}

static void pollevent()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff;

    call_walk(CALL_L1, CALL_DR, 15, "system/event/wm");
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

                if (data[0] == 0x10)
                    sendevent(1001);

                if (data[0] == 0x19)
                    spawn();

                if (data[0] == 0x24)
                    nextwindow(viewactive);

                if (data[0] == 0x25)
                    prevwindow(viewactive);

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
        box_setsize(&view[i].panel.size, SCREEN_BORDER + i * 16, SCREEN_BORDER, 16, 16);
        list_add(&views, &view[i].item);

    }

    viewactive = views.head->data;
    viewactive->active = 1;
    viewactive->panel.active = 1;

}

void main()
{

    setupwindows();
    setupviews();
    box_setsize(&back, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    box_setsize(&empty, SCREEN_BORDER + 1, 16 + SCREEN_BORDER + 1, SCREEN_WIDTH - 2 * SCREEN_BORDER - 2, SCREEN_HEIGHT - 16 - 2 * SCREEN_BORDER - 2);
    panel_init(&field, "1", 0);
    panel_init(&clock, "1", 0);
    box_setsize(&field.size, 64 + SCREEN_BORDER, SCREEN_BORDER, 216 - SCREEN_BORDER, 16);
    box_setsize(&clock.size, 280, SCREEN_BORDER, 40 - SCREEN_BORDER, 16);
    draw_setmode();
    draw_setcolormap();
    draw_begin();
    backbuffer_fillbox(&back, 0);
    draw_end();
    draw();
    pollevent();

}

