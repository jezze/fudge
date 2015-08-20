#include <abi.h>
#include <fudge.h>
#include <format/pcf.h>
#include "box.h"
#include "renderable.h"
#include "draw.h"
#include "glyph.h"

static void render(struct renderable *self, struct ctrl_videosettings *settings, unsigned int line)
{

    struct glyph *glyph = self->data;
    unsigned int offset = (line - self->size.y) * glyph->padding;
    unsigned int x;

    for (x = 0; x < glyph->padding; x++)
    {

        unsigned char c = glyph->data[offset + x];
        unsigned int k = 0;
        unsigned char i;

        for (i = 0x80; i; i >>= 1)
        {

            if (c & i)
                draw_fill(settings->bpp, glyph->color, glyph->base.size.x + x * 8 + k, 1);

            k++;

        }

    }

}

void glyph_assign(struct glyph *glyph, void *data, unsigned short encoding)
{

    unsigned short index = pcf_getindex(data, encoding);
    struct pcf_metricsdata metricsdata;

    glyph->padding = pcf_getpadding(data);
    glyph->data = pcf_getbitmapdata(data) + pcf_getbitmapoffset(data, index);

    pcf_readmetricsdata(data, encoding, &metricsdata);

    glyph->base.size.w = metricsdata.width;
    glyph->base.size.h = metricsdata.ascent + metricsdata.descent;

}

void glyph_init(struct glyph *glyph, unsigned int color)
{

    renderable_init(&glyph->base, glyph, render);

    glyph->color = color;

}

