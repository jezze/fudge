#include "memory.h"
#include "ctrl.h"

void ctrl_init_header(struct ctrl_header *header, unsigned int type)
{

    memory_clear(header, sizeof (struct ctrl_header));

    header->type = type;

}

void ctrl_init_videoctrl(struct ctrl_videoctrl *videoctrl, unsigned int w, unsigned int h, unsigned int bpp)
{

    memory_clear(videoctrl, sizeof (struct ctrl_videoctrl));
    ctrl_init_header(&videoctrl->header, CTRL_TYPE_VIDEOCTRL);

    videoctrl->w = w;
    videoctrl->h = h;
    videoctrl->bpp = bpp;

}

