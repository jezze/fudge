#include <abi.h>
#include <fudge.h>
#include "box.h"

void box_setsize(struct box *box, int x, int y, int w, int h)
{

    box->x = x;
    box->y = y;
    box->w = w;
    box->h = h;

}

