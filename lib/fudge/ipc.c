#include "ipc.h"

void ipc_init(struct ipc_header *header, unsigned int type, unsigned int length)
{

    header->type = type;
    header->source = 0;
    header->target = 0;
    header->length = sizeof (struct ipc_header) + length;

}

