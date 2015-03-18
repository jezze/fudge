#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "draw.h"
#include "text.h"
#include "window.h"

void window_draw(struct window *window, unsigned int line)
{

    unsigned int color;

    if (line < window->size.y || line >= window->size.y + window->size.h)
        return;

    if (window->active)
        color = WM_COLOR_ACTIVEFRAME;
    else
        color = WM_COLOR_PASSIVEFRAME;

    if (line < window->size.y + 2 || line >= window->size.y + window->size.h - 2)
    {

        backbuffer_fillcount(color, window->size.x, window->size.w);

    }

    else
    {

        backbuffer_fillcount(color, window->size.x, 2);
        backbuffer_fillcount(color, window->size.x + window->size.w - 2, 2);

    }

}

void window_init(struct window *window, char *text, unsigned int active)
{

    list_inititem(&window->item, window);
    box_setsize(&window->size, 0, 0, 0, 0);

    window->active = active;
    window->title.content = text;

}

