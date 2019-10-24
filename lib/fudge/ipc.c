#include "ipc.h"

unsigned int ipc_addroute(struct ipc_header *header, unsigned int target)
{

    if (header->nroutes < 16)
    {

        header->routes[header->nroutes] = target;
        header->nroutes++;

    }

    return header->nroutes;

}

void ipc_create(struct ipc_header *header, unsigned int type, unsigned int length)
{

    header->type = type;
    header->source = 0;
    header->target = 0;
    header->length = sizeof (struct ipc_header) + length;
    header->nroutes = 0;

}

