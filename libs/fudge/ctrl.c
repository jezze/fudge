#include "memory.h"
#include "ctrl.h"

void ctrl_init_header(struct ctrl_header *header, unsigned int type)
{

    memory_clear(header, sizeof (struct ctrl_header));

    header->type = type;

}

void ctrl_init_consolesettings(struct ctrl_consolesettings *settings)
{

    memory_clear(settings, sizeof (struct ctrl_consolesettings));
    ctrl_init_header(&settings->header, CTRL_TYPE_CONSOLE);

}

void ctrl_init_networksettings(struct ctrl_networksettings *settings)
{

    memory_clear(settings, sizeof (struct ctrl_networksettings));
    ctrl_init_header(&settings->header, CTRL_TYPE_NETWORK);

}

void ctrl_init_videosettings(struct ctrl_videosettings *settings)
{

    memory_clear(settings, sizeof (struct ctrl_videosettings));
    ctrl_init_header(&settings->header, CTRL_TYPE_VIDEO);

}

