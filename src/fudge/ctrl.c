#include "ctrl.h"

void ctrl_setheader(struct ctrl_header *header, unsigned int type)
{

    header->type = type;

}

void ctrl_setconsolesettings(struct ctrl_consolesettings *settings, unsigned char scroll)
{

    ctrl_setheader(&settings->header, CTRL_TYPE_CONSOLE);

    settings->scroll = scroll;

}

void ctrl_setvideosettings(struct ctrl_videosettings *settings, unsigned int w, unsigned int h, unsigned int bpp)
{

    ctrl_setheader(&settings->header, CTRL_TYPE_VIDEO);

    settings->w = w;
    settings->h = h;
    settings->bpp = bpp;

}

