#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"
#include "draw.h"
#include "glyph.h"

static void render(struct renderable *self, struct ctrl_videosettings *settings, unsigned int line)
{

    struct glyph *glyph = self->data;
    unsigned int i;
    int off;

    if (line < self->size.y || line >= self->size.y + self->size.h)
        return;

    off = (line - self->size.y) * self->size.w;

    for (i = 0; i < self->size.w; i++)
    {

        if (glyph->data[off + i] != 0xFF)
            draw_fill(settings->bpp, glyph->data[off + i], self->size.x + i, 1);

    }

}

void glyph_init(struct glyph *glyph, void *data)
{

    renderable_init(&glyph->base, glyph, render);

    glyph->data = data;

}

