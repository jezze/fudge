#include "box.h"

unsigned int box_isinside(struct box *box, unsigned int x, unsigned int y)
{

    return (box->x <= x && box->y <= y && box->x + box->w > x && box->y + box->h > y);

}

void box_setsize(struct box *box, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    box->x = x;
    box->y = y;
    box->w = w;
    box->h = h;

}

void box_resize(struct box *box, int f)
{

    box->x += f;
    box->y += f;
    box->w -= f * 2;
    box->h -= f * 2;

}

