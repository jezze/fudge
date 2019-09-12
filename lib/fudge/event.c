#include "define.h"
#include "memory.h"
#include "event.h"

unsigned int event_addroute(struct event_header *header, unsigned int target, unsigned int session)
{

    if (header->nroutes < 16)
    {

        header->routes[header->nroutes].target = target;
        header->routes[header->nroutes].session = session;
        header->nroutes++;

    }

    return header->nroutes;

}

void event_create(struct event_header *header, unsigned int type, unsigned int length)
{

    header->type = type;
    header->source = 0;
    header->target = 0;
    header->session = 0;
    header->length = sizeof (struct event_header) + length;
    header->nroutes = 0;

}

