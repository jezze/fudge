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

