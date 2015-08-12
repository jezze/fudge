#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "draw.h"
#include "image.h"

void image_draw(struct image *image, struct ctrl_videosettings *settings, unsigned int line)
{

    unsigned int i;
    int off;

    if (line < image->size.y || line >= image->size.y + image->size.h)
        return;

    off = (line - image->size.y) * image->size.w;

    for (i = 0; i < image->size.w; i++)
    {

        if (image->data[off + i] != 0xFF)
            draw_fill(settings->bpp, image->data[off + i], image->size.x + i, 1);

    }

}

void image_init(struct image *image, void *data)
{

    image->data = data;

}

