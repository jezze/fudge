#include "buffer.h"
#include "cstring.h"
#include "message.h"

unsigned int message_datasize(struct message_header *header)
{

    return header->length - sizeof (struct message_header);

}

void message_initheader(struct message_header *header, unsigned int event, unsigned int length)
{

    header->event = event;
    header->source = 0;
    header->length = sizeof (struct message_header) + length;

}

