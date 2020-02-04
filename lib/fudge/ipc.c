#include "ipc.h"

unsigned int ipc_datasize(struct ipc_header *header)
{

    return header->length - sizeof (struct ipc_header);

}

void ipc_init(struct ipc_header *header, unsigned int type, unsigned int length)
{

    header->type = type;
    header->source = 0;
    header->length = sizeof (struct ipc_header) + length;

}

