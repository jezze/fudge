#include "event.h"

void event_initheader(struct event_header *header, unsigned int type, unsigned int source, unsigned int destination, unsigned int length)
{

    header->type = type;
    header->source = source;
    header->destination = destination;
    header->length = sizeof (struct event_header) + length;

}

