#include <fudge.h>
#include <abi.h>

static unsigned int onstop(struct event_channel *channel)
{

    event_reply(channel, EVENT_DATA);
    event_append(&channel->o, 13, "Hello world!\n");
    event_place(channel->o.header.target, &channel->o);

    return 1;

}

static unsigned int onkill(struct event_channel *channel)
{

    return 1;

}

void main(void)
{

    unsigned int status = 0;
    struct event_channel channel;

    while (!status)
    {

        switch (event_pick(&channel))
        {

        case EVENT_STOP:
            status = onstop(&channel);

            break;

        case EVENT_KILL:
            status = onkill(&channel);

            break;

        }

    }

}

