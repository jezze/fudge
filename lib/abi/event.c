#include <fudge.h>
#include "call.h"
#include "event.h"

#define EVENTS                          64

static unsigned int (*signals[EVENTS])(struct event_channel *channel);

static unsigned int ignore(struct event_channel *channel)
{

    return 0;

}

static unsigned int abort(struct event_channel *channel)
{

    return 1;

}

unsigned int event_place(unsigned int id, union event_message *message)
{

    while (!call_place(id, message, message->header.length));

    return message->header.type;

}

unsigned int event_listen(void)
{

    struct event_channel channel;

    while (!call_pick(&channel.i.header, sizeof (struct event_header)));

    if (channel.i.header.length > sizeof (struct event_header))
        while (!call_pick(&channel.i.header + 1, channel.i.header.length - sizeof (struct event_header)));

    if (signals[channel.i.header.type])
    {

        if (signals[channel.i.header.type](&channel))
            call_despawn();

        return channel.i.header.type;

    }

    return 0;

}

void event_setsignal(unsigned int type, unsigned int (*callback)(struct event_channel *channel))
{

    signals[type] = callback;

}

void event_clearsignal(unsigned int type)
{

    signals[type] = 0;

}

void event_initsignals(void)
{

    unsigned int i;

    for (i = 0; i < EVENTS; i++)
        event_setsignal(i, ignore);

    event_setsignal(EVENT_KILL, abort);
    event_setsignal(EVENT_STOP, abort);

}

