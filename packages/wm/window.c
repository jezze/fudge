#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "draw.h"
#include "text.h"
#include "window.h"

void window_draw(struct window *window, unsigned int line)
{

    if (line < window->size.y || line >= window->size.y + window->size.h)
        return;

    if (line == window->size.y + 0 || line >= window->size.y + window->size.h - 1)
    {

        fill(WM_COLOR_DARK, window->size.x, window->size.w);

    }

    else if (line == window->size.y + 1 || line >= window->size.y + window->size.h - 2)
    {

        fill(WM_COLOR_DARK, window->size.x + 0, 1);
        fill(WM_COLOR_DARK, window->size.x + window->size.w - 1, 1);
        fill(window->active ? WM_COLOR_ACTIVEFRAME : WM_COLOR_PASSIVEFRAME, window->size.x + 1, window->size.w - 2);

    }

    else if (line == window->size.y + 2 || line >= window->size.y + window->size.h - 3)
    {

        fill(WM_COLOR_DARK, window->size.x + 0, 1);
        fill(WM_COLOR_DARK, window->size.x + window->size.w - 1, 1);
        fill(window->active ? WM_COLOR_ACTIVEFRAME : WM_COLOR_PASSIVEFRAME, window->size.x + 1, 1);
        fill(window->active ? WM_COLOR_ACTIVEFRAME : WM_COLOR_PASSIVEFRAME, window->size.x + window->size.w - 2, 1);
        fill(WM_COLOR_DARK, window->size.x + 2, window->size.w - 4);

    }

    else
    {

        fill(WM_COLOR_DARK, window->size.x + 0, 1);
        fill(WM_COLOR_DARK, window->size.x + window->size.w - 1, 1);
        fill(window->active ? WM_COLOR_ACTIVEFRAME : WM_COLOR_PASSIVEFRAME, window->size.x + 1, 1);
        fill(window->active ? WM_COLOR_ACTIVEFRAME : WM_COLOR_PASSIVEFRAME, window->size.x + window->size.w - 2, 1);
        fill(WM_COLOR_DARK, window->size.x + 2, 1);
        fill(WM_COLOR_DARK, window->size.x + window->size.w - 3, 1);

    }

}

void window_init(struct window *window, char *text, unsigned int active)
{

    list_inititem(&window->item, window);
    box_setsize(&window->size, 0, 0, 0, 0);

    window->active = active;
    window->title.content = text;

}

