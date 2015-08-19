#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"

void renderable_render(struct renderable *renderable, struct ctrl_videosettings *settings, unsigned int line)
{

    if (line < renderable->size.y || line >= renderable->size.y + renderable->size.h)
        return;

    if (!renderable->visible)
        return;

    renderable->render(renderable, settings, line);

}

void renderable_init(struct renderable *renderable, void *data, void (*render)(struct renderable *self, struct ctrl_videosettings *settings, unsigned int line))
{

    list_inititem(&renderable->item, renderable);

    renderable->data = data;
    renderable->render = render;

}

