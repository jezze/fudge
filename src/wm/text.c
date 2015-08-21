#include <abi.h>
#include <fudge.h>
#include <format/pcf.h>
#include "box.h"
#include "renderable.h"
#include "draw.h"
#include "text.h"

void text_render(struct renderable *self, struct ctrl_videosettings *settings, unsigned int line)
{

    struct text *text = self->data;
    unsigned int padding = pcf_getpadding(text->data);
    struct box size;
    unsigned int i;

    size.x = text->base.size.x;
    size.y = text->base.size.y;

    for (i = 0; i < text->count; i++)
    {

        unsigned short index = pcf_getindex(text->data, text->string[i]);
        unsigned int offset = (line - size.y) * padding;
        unsigned char *data;
        struct pcf_metricsdata metricsdata;
        unsigned int x;

        pcf_readmetricsdata(text->data, index, &metricsdata);

        data = pcf_getbitmapdata(text->data) + pcf_getbitmapoffset(text->data, index);
        size.w = metricsdata.width;
        size.h = metricsdata.ascent + metricsdata.descent;

        for (x = 0; x < padding; x++)
        {

            unsigned char c = data[offset + x];
            unsigned int k = 0;
            unsigned char b;

            for (b = 0x80; b; b >>= 1)
            {

                if (c & b)
                    draw_fill(settings->bpp, text->color, size.x + x * 8 + k, 1);

                k++;
                
            }

        }

        size.x += size.w;

    }

}

void text_assign(struct text *text, void *data, unsigned int count, char *string)
{

    text->data = data;
    text->count = count;
    text->string = string;

}

void text_init(struct text *text, unsigned int color)
{

    renderable_init(&text->base, RENDERABLE_TYPE_TEXT, sizeof (struct text) - sizeof (struct renderable), text);

    text->color = color;

}

