#include <abi.h>
#include <fudge.h>
#include <format/pcf.h>
#include "box.h"
#include "renderable.h"
#include "draw.h"
#include "glyph.h"
#include "text.h"

static void render(struct renderable *self, struct ctrl_videosettings *settings, unsigned int line)
{

    struct text *text = self->data;
    unsigned int i;

    text->glyph.base.size.x = text->base.size.x;
    text->glyph.base.size.y = text->base.size.y;

    for (i = 0; i < text->count; i++)
    {

        glyph_assign(&text->glyph, text->data, text->string[i]);
        renderable_render(&text->glyph.base, settings, line);

        text->glyph.base.size.x += text->glyph.base.size.w;

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

    renderable_init(&text->base, text, render);
    glyph_init(&text->glyph, color);

    text->glyph.base.visible = 1;

}

