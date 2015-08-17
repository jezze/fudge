#include <abi.h>
#include <fudge.h>
#include <format/pcf.h>
#include "box.h"
#include "renderable.h"
#include "draw.h"
#include "glyph.h"

static void renderbyte(struct glyph *glyph, struct ctrl_videosettings *settings, unsigned char c)
{

    unsigned char i = 0x80;
    unsigned int k = 0;

    do
    {

        if (c & i)
            draw_fill(settings->bpp, glyph->color, glyph->base.size.x + k, 1);

        k++;

    } while (i >>= 1);

}

static void render(struct renderable *self, struct ctrl_videosettings *settings, unsigned int line)
{

    struct glyph *glyph = self->data;
    unsigned int x;
    unsigned int y;

    if (line < self->size.y || line >= self->size.y + self->size.h)
        return;

    y = line - self->size.y;

    for (x = 0; x < glyph->padding; x++)
        renderbyte(glyph, settings, glyph->data[y * glyph->padding + x]);

}

void glyph_assign(struct glyph *glyph, void *data, unsigned short encoding)
{

    unsigned short index = pcf_getindex(data, encoding);
    struct pcf_metricsdata metricsdata;

    glyph->padding = pcf_getpadding(data);
    glyph->data = pcf_getbitmapdata(data) + pcf_getbitmapoffset(data, index);

    pcf_getmetricsdata(data, encoding, &metricsdata);

    glyph->base.size.w = metricsdata.width;
    glyph->base.size.h = metricsdata.ascent + metricsdata.descent;

}

void glyph_init(struct glyph *glyph, unsigned int color)
{

    renderable_init(&glyph->base, glyph, render);

    glyph->color = color;

}

