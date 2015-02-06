#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "draw.h"
#include "text.h"
#include "window.h"

void window_draw(struct window *window)
{

    struct box frameborder;
    struct box frame;
    struct box bodyborder;
    struct box body;

    box_setsize(&frameborder, window->size.x, window->size.y, window->size.w, window->size.h);
    box_setsize(&frame, frameborder.x + 1, frameborder.y + 1, frameborder.w - 2, frameborder.h - 2);
    box_setsize(&bodyborder, frame.x + 1, frame.y + 1, frame.w - 2, frame.h - 2);
    box_setsize(&body, bodyborder.x + 1, bodyborder.y + 1, bodyborder.w - 2, bodyborder.h - 2);
    backbuffer_fillbox(&frameborder, WM_COLOR_DARK);

    if (window->active)
        backbuffer_fillbox(&frame, WM_COLOR_ACTIVEFRAME);
    else
        backbuffer_fillbox(&frame, WM_COLOR_PASSIVEFRAME);

    backbuffer_fillbox(&bodyborder, WM_COLOR_DARK);
    backbuffer_fillbox(&body, WM_COLOR_BODY);

}

void window_init(struct window *window, char *text, unsigned int active)
{

    list_inititem(&window->item, window);
    box_setsize(&window->size, 0, 0, 0, 0);

    window->active = active;
    window->title.content = text;

}

