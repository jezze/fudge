#include <fudge.h>
#include <abi.h>

static unsigned int (*signals[EVENTS])(struct event_channel *channel);

static unsigned int onstop(struct event_channel *channel)
{

    unsigned int id;

    if (!file_walk2(FILE_CP, "/bin/echo"))
        return 0;

    if (!file_walk2(FILE_C0, "/data/motd.txt"))
        return 0;

    id = call_spawn();

    if (!id)
        return 0;

    event_forward(channel, EVENT_INIT);
    event_place(id, channel);
    event_forward(channel, EVENT_FILE);
    event_addfile(&channel->o, FILE_P0);
    event_place(id, channel);
    event_forward(channel, EVENT_STOP);
    event_place(id, channel);

    return 1;

}

void main(void)
{

    struct event_channel channel;

    event_initsignals(signals);
    event_setsignal(signals, EVENT_STOP, onstop);

    while (event_listen(signals, &channel));

}

