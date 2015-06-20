#include "ctrl.h"

void ctrl_initheader(struct ctrl_header *header, unsigned int type)
{

    header->type = type;

}

void ctrl_initconsolesettings(struct ctrl_consolesettings *settings, unsigned char scroll)
{

    ctrl_initheader(&settings->header, CTRL_TYPE_CONSOLE);

    settings->scroll = scroll;

}

void ctrl_initvideosettings(struct ctrl_videosettings *settings, unsigned int w, unsigned int h, unsigned int bpp)
{

    ctrl_initheader(&settings->header, CTRL_TYPE_VIDEO);

    settings->w = w;
    settings->h = h;
    settings->bpp = bpp;

}

