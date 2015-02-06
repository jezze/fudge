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
static unsigned int windows;
static struct view view[VIEWS];
static unsigned int views;
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

static void draw()
{

    unsigned int i;

    draw_begin();
    panel_draw(&field);
    panel_draw(&clock);

    for (i = 0; i < views; i++)
    {

        struct view *v = &view[i];

        panel_draw(&v->panel);

        if (!v->active)
            continue;

        if (v->windows.head)
            drawwindows(&v->windows);
        else
            backbuffer_fillbox(&empty, WM_COLOR_BODY);

    }

    draw_end();

}

static struct window *createwindow()
{

    window_init(&window[windows], "1212", 0);

    windows++;

    return &window[windows - 1];

}

static void sortwindows(struct view *view)
{

    unsigned int count = list_count(&view->windows);
    struct list_item *current = view->windows.head;
    struct window *window;

    if (!count)
        return;

    window = current->data;

    if (count == 1)
    {

        box_setsize(&window->size, 1, 18, 318, 181);

        return;

    }

    box_setsize(&window->size, 1, 18, 159, 181);

    current = current->next;
    window = current->data;

    box_setsize(&window->size, 160, 18, 159, 181);

}

static void activatewindow()
{

    struct window *window = createwindow();

    view_addwindow(viewactive, window);
    sortwindows(viewactive);

    if (viewactive->windowactive == window)
        return;

    if (viewactive->windowactive)
    {

        viewactive->windowactive->active = 0;

    }

    viewactive->windowactive = window;
    viewactive->windowactive->active = 1;

    draw();

}

static void activateview(struct view *v)
{

    if (viewactive == v)
        return;

    if (viewactive)
    {

        viewactive->active = 0;
        viewactive->panel.active = 0;

    }

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

                break;

            case 1000:
                    activatewindow();

                break;

            }

        }

    }

    call_close(CALL_L1);

}

void main()
{

    windows = 0;
    views = 4;
    viewactive = &view[0];

    view_init(&view[0], "1", 1);
    view_init(&view[1], "2", 0);
    view_init(&view[2], "3", 0);
    view_init(&view[3], "4", 0);
    panel_init(&field, "1", 0);
    panel_init(&clock, "1", 0);
    box_setsize(&back, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    box_setsize(&empty, 2, 19, 316, 179);
    box_setsize(&view[0].panel.size, 1, 1, 17, 17);
    box_setsize(&view[1].panel.size, 18, 1, 17, 17);
    box_setsize(&view[2].panel.size, 35, 1, 17, 17);
    box_setsize(&view[3].panel.size, 52, 1, 17, 17);
    box_setsize(&field.size, 69, 1, 212, 17);
    box_setsize(&clock.size, 281, 1, 38, 17);
    draw_setmode();
    draw_setcolormap();
    draw_begin();
    backbuffer_fillbox(&back, 0);
    draw_end();
    draw(&back);
    pollevent();

}

