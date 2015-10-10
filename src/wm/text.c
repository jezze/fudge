#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"
#include "text.h"

void text_assign(struct text *text, unsigned int count, char *string)
{

    text->count = count;
    text->string = string;
    text->base.header.count += count;

}

void text_init(struct text *text, unsigned int type)
{

    renderable_init(&text->base, RENDERABLE_TYPE_TEXT, sizeof (struct text_header));

    text->header.type = type;

}

