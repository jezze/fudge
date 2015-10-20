#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"

void renderable_init(struct renderable *renderable, unsigned int type, unsigned int count)
{

    list_inititem(&renderable->item, renderable);

    renderable->header.id = (unsigned int)renderable;
    renderable->header.type = type;
    renderable->header.count = count;
    renderable->header.z = 1;

}

