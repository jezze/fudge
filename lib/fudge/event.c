#include "event.h"

void event_initheader(struct event_header *header, unsigned int type, unsigned int source, unsigned int target, unsigned int length)
{

    header->type = type;
    header->source = source;
    header->target = target;
    header->length = sizeof (struct event_header) + length;

}

