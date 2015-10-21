#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"

void renderable_init(struct renderable *renderable, unsigned int type, unsigned int count)
{

    renderable->id = (unsigned int)renderable;
    renderable->type = type;
    renderable->count = count;
    renderable->z = 1;

}

void renderable_initmouse(struct renderable_mouse *mouse)
{

    renderable_init(&mouse->base, RENDERABLE_TYPE_MOUSE, 0);

    mouse->base.z = 2;

}

void renderable_initpanel(struct renderable_panel *panel)
{

    renderable_init(&panel->base, RENDERABLE_TYPE_PANEL, sizeof (struct renderable_panelheader));

}

void renderable_inittext(struct renderable_text *text, unsigned int type)
{

    renderable_init(&text->base, RENDERABLE_TYPE_TEXT, sizeof (struct renderable_textheader));

    text->header.type = type;

}

void renderable_initwindow(struct renderable_window *window)
{

    renderable_init(&window->base, RENDERABLE_TYPE_WINDOW, sizeof (struct renderable_windowheader));

}

