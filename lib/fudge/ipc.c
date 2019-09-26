#include "ipc.h"

unsigned int ipc_addroute(struct ipc_header *header, unsigned int target, unsigned int session)
{

    if (header->nroutes < 16)
    {

        header->routes[header->nroutes].target = target;
        header->routes[header->nroutes].session = session;
        header->nroutes++;

    }

    return header->nroutes;

}

void ipc_create(struct ipc_header *header, unsigned int type, unsigned int length)
{

    header->type = type;
    header->source = 0;
    header->target = 0;
    header->session = 0;
    header->length = sizeof (struct ipc_header) + length;
    header->nroutes = 0;

}

