#include <fudge.h>
#include <abi.h>

static void ontimertick(struct channel *channel, void *mdata, unsigned int msize)
{

}

void setup(struct channel *channel)
{

    struct ctrl_timertick timertick;

    timertick.jiffies = 1000;
    timertick.repeat = 1;

    channel_request(channel, EVENT_CTRL);
    channel_append(channel, sizeof (struct ctrl_timertick), &timertick);
    channel_write(channel, FILE_G1);

}

void main(void)
{

    struct channel channel;

    if (!file_walk2(FILE_G0, "/system/timer/if:0/event"))
        return;

    if (!file_walk2(FILE_G1, "/system/timer/if:0/ctrl"))
        return;

    file_open(FILE_G0);
    channel_init(&channel);
    channel_setsignal(&channel, EVENT_TIMERTICK, ontimertick);
    setup(&channel);
    channel_listen(&channel);
    file_close(FILE_G0);

}

