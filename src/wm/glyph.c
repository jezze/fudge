#include <abi.h>
#include <fudge.h>
#include <format/pcf.h>
#include "box.h"
#include "renderable.h"
#include "draw.h"
#include "glyph.h"

static void writebyte(struct glyph *glyph, struct ctrl_videosettings *settings, unsigned char c)
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

static void writechar(struct glyph *glyph, struct ctrl_videosettings *settings, unsigned int y)
{

    unsigned int padding = pcf_getpadding(glyph->data);
    unsigned short index = pcf_getindex(glyph->data, glyph->value);
    unsigned int offset = pcf_getbitmapoffset(glyph->data, index);
    unsigned char *data = pcf_getbitmapdata(glyph->data);
    struct pcf_metricsdata metrics;
    unsigned int x;

    pcf_getmetricsdata(glyph->data, index, &metrics);

    for (x = 0; x < padding; x++)
        writebyte(glyph, settings, data[offset + y * padding + x]);

}

static void render(struct renderable *self, struct ctrl_videosettings *settings, unsigned int line)
{

    struct glyph *glyph = self->data;

    if (line < self->size.y || line >= self->size.y + self->size.h)
        return;

    writechar(glyph, settings, line - self->size.y);

}

void glyph_init(struct glyph *glyph, void *data)
{

    renderable_init(&glyph->base, glyph, render);

    glyph->data = data;
    glyph->value = '0';
    glyph->color = WM_COLOR_LIGHT;

}

