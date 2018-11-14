#include <fudge.h>
#include "call.h"
#include "event.h"

unsigned int event_pick(union event_message *message)
{

    while (!call_pick(&message->header, sizeof (struct event_header)));

    if (message->header.length > sizeof (struct event_header))
        while (!call_pick(&message->header + 1, message->header.length - sizeof (struct event_header)));

    return message->header.type;

}

unsigned int event_place(unsigned int id, union event_message *message)
{

    while (!call_place(id, message, message->header.length));

    return message->header.length;

}

