#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"
#include "mouse.h"

void mouse_init(struct mouse *mouse)
{

    renderable_init(&mouse->base, RENDERABLE_TYPE_MOUSE, 0);

    mouse->base.z = 2;

}

