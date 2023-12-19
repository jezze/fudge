#include <fudge.h>
#include <abi.h>
#include "config.h"
#include "util.h"
#include "text.h"
#include "widget.h"

static unsigned int cmapbutton[13] = {
    0xE8101010, 0xE8484848, 0xE8404040, 0xE8888888,
    0xE8101010, 0xE8505050, 0xE8484848, 0xE8888888,
    0xE8101010, 0xE8585858, 0xE8505050, 0xE8888888,
    0xE8FFFFFF,
};

static unsigned int cmapchoice[13] = {
    0xE8101010, 0xE8484848, 0xE8484848, 0xE8888888,
    0xE8101010, 0xE8505050, 0xE8505050, 0xE8888888,
    0xE8101010, 0xE8585858, 0xE8585858, 0xE8888888,
    0xE8FFFFFF,
};

static unsigned int cmaplistbox[12] = {
    0xE8101010, 0xE82A2A2A, 0xE8101010, 0xE8686868,
    0xE8101010, 0xE82A2A2A, 0xE8101010, 0xE8787878,
    0xE8101010, 0xE82A2A2A, 0xE8101010, 0xE8888888,
};

static unsigned int cmapmouse[3] = {
    0xE8101010, 0xE8202020, 0xE8F0F0F0,
};

static unsigned int cmapselect[14] = {
    0xE8101010, 0xE82A2A2A, 0xE8101010, 0xE8686868,
    0xE8101010, 0xE82A2A2A, 0xE8101010, 0xE8787878,
    0xE8101010, 0xE8805050, 0xE8805050, 0xE8E0B0B0,
    0xE8FFFFFF,
    0xE8FFFFFF,
};

static unsigned int cmaptext[1] = {
    0xE8E0E0E0,
};

static unsigned int cmaptextbox[16] = {
    0xE8101010, 0xE82A2A2A, 0xE8101010, 0xE8686868,
    0xE8101010, 0xE82A2A2A, 0xE8101010, 0xE8787878,
    0xE8101010, 0xE82A2A2A, 0xE8101010, 0xE8888888,
    0xE8101010, 0xE82A2A2A, 0xE8101010, 0xE8686868,
};

static unsigned int cmaptextbutton[13] = {
    0x00242424, 0x00242424, 0x00242424, 0x00242424,
    0xE8242424, 0xE8242424, 0xE8242424, 0xE8242424,
    0xE8242424, 0xE8242424, 0xE8242424, 0xE8242424,
    0xE8FFFFFF,
};

static unsigned int cmapwindow[12] = {
    0xE8101010, 0xE8805050, 0xE8805050, 0xE8E0B0B0,
    0xE8101010, 0xE8303030, 0xE8303030, 0xE8585858,
    0xA8FFFFFF,
    0xE8FFFFFF,
    0xE8FFFFFF,
    0xE8FFFFFF,
};

static unsigned int *getcmap(unsigned int type)
{

    switch (type)
    {

    case WIDGET_TYPE_BUTTON:
        return cmapbutton;

    case WIDGET_TYPE_CHOICE:
        return cmapchoice;

    case WIDGET_TYPE_LISTBOX:
        return cmaplistbox;

    case WIDGET_TYPE_IMAGE:
        return cmapmouse;

    case WIDGET_TYPE_SELECT:
        return cmapselect;

    case WIDGET_TYPE_TEXT:
        return cmaptext;

    case WIDGET_TYPE_TEXTBOX:
        return cmaptextbox;

    case WIDGET_TYPE_TEXTBUTTON:
        return cmaptextbutton;

    case WIDGET_TYPE_WINDOW:
        return cmapwindow;

    }

    return 0;

}

unsigned int *cmap_get(unsigned int state, unsigned int type, unsigned int offset, unsigned int step)
{

    unsigned int *cmap = getcmap(type);

    switch (state)
    {

    case WIDGET_STATE_FOCUS:
        return cmap + offset + step * 2;

    case WIDGET_STATE_HOVER:
        return cmap + offset + step * 1;

    case WIDGET_STATE_NORMAL:
    default:
        return cmap + offset;

    }

}

