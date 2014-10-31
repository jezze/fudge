#include "memory.h"
#include "ctrl.h"

void ctrl_init_header(struct ctrl_header *header, unsigned int type)
{

    memory_clear(header, sizeof (struct ctrl_header));

    header->type = type;

}

void ctrl_init_networkctrl(struct ctrl_networkctrl *networkctrl)
{

    memory_clear(networkctrl, sizeof (struct ctrl_networkctrl));
    ctrl_init_header(&networkctrl->header, CTRL_TYPE_NETWORKCTRL);

}

void ctrl_init_videoctrl(struct ctrl_videoctrl *videoctrl)
{

    memory_clear(videoctrl, sizeof (struct ctrl_videoctrl));
    ctrl_init_header(&videoctrl->header, CTRL_TYPE_VIDEOCTRL);

}

