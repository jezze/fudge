#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"
#include "window.h"

void window_init(struct window *window)
{

    renderable_init(&window->base, RENDERABLE_TYPE_WINDOW, sizeof (struct window_header));

}

