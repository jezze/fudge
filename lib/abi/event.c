#include <fudge.h>
#include "call.h"
#include "event.h"

unsigned int event_pick(struct event_channel *channel)
{

    while (!call_pick(&channel->i.header, sizeof (struct event_header)));

    if (channel->i.header.length > sizeof (struct event_header))
        while (!call_pick(&channel->i.header + 1, channel->i.header.length - sizeof (struct event_header)));

    return channel->i.header.type;

}

unsigned int event_place(unsigned int id, union event_message *message)
{

    while (!call_place(id, message, message->header.length));

    return message->header.type;

}

