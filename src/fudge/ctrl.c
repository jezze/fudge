#include "ctrl.h"

void ctrl_setheader(struct ctrl_header *header, unsigned int type)
{

    header->type = type;

}

void ctrl_setclocksettings(struct ctrl_clocksettings *settings, unsigned char seconds, unsigned char minutes, unsigned char hours, unsigned char weekday, unsigned char day, unsigned char month, unsigned short year)
{

    ctrl_setheader(&settings->header, CTRL_TYPE_CLOCK);

    settings->seconds = seconds;
    settings->minutes = minutes;
    settings->hours = hours;
    settings->weekday = weekday;
    settings->day = day;
    settings->month = month;
    settings->year = year;

}

void ctrl_setconsettings(struct ctrl_consettings *settings, unsigned int interface, unsigned int protocol, unsigned int port)
{

    ctrl_setheader(&settings->header, CTRL_TYPE_CON);

    settings->interface = interface;
    settings->port = port;

}

void ctrl_setconsolesettings(struct ctrl_consolesettings *settings, unsigned char scroll)
{

    ctrl_setheader(&settings->header, CTRL_TYPE_CONSOLE);

    settings->scroll = scroll;

}

void ctrl_setpartsettings(struct ctrl_partsettings *settings, unsigned int interface, unsigned int start, unsigned int end)
{

    ctrl_setheader(&settings->header, CTRL_TYPE_PART);

    settings->interface = interface;
    settings->start = start;
    settings->end = end;

}

void ctrl_setvideosettings(struct ctrl_videosettings *settings, unsigned int w, unsigned int h, unsigned int bpp)
{

    ctrl_setheader(&settings->header, CTRL_TYPE_VIDEO);

    settings->w = w;
    settings->h = h;
    settings->bpp = bpp;

}

