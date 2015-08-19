#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"
#include "draw.h"
#include "image.h"

static void render(struct renderable *self, struct ctrl_videosettings *settings, unsigned int line)
{

    struct image *image = self->data;
    unsigned int offset = (line - self->size.y) * self->size.w;
    unsigned int i;

    for (i = 0; i < self->size.w; i++)
    {

        if (image->data[offset + i] != 0xFF)
            draw_fill(settings->bpp, image->data[offset + i], self->size.x + i, 1);

    }

}

void image_init(struct image *image, void *data)
{

    renderable_init(&image->base, image, render);

    image->data = data;

}

