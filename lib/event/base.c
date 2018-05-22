#include <abi.h>
#include <fudge.h>
#include "base.h"

unsigned int event_read(struct event *event, unsigned int descriptor)
{

    unsigned int count = file_readall(descriptor, &event->header, sizeof (struct event_header));

    if (!count)
        return 0;

    if (event->header.length - count > 0)
        count += file_readall(descriptor, event->data, event->header.length - count);

    return count;

}

void event_send(unsigned int descriptor, unsigned int destination, unsigned int type, void *buffer, unsigned int length)
{

    struct event_header *header = buffer;

    header->destination = destination;
    header->type = type;
    header->length = length;

    file_writeall(descriptor, header, header->length);

}

