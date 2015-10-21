#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"
#include "text.h"

void text_init(struct text *text, unsigned int type)
{

    renderable_init(&text->base, RENDERABLE_TYPE_TEXT, sizeof (struct text_header));

    text->header.type = type;

}

