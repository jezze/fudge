#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"

void renderable_init(struct renderable *renderable, unsigned int type, unsigned int count)
{

    renderable->id = (unsigned int)renderable;
    renderable->type = type;
    renderable->count = count;
    renderable->z = 1;

}

