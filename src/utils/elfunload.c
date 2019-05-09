#include <fudge.h>
#include <abi.h>

static unsigned int (*signals[EVENTS])(struct event_channel *channel);

static unsigned int onfile(struct event_channel *channel)
{

    struct event_file *file = event_getdata(channel);

    file_open(file->descriptor);
    call_unload(file->descriptor);
    file_close(file->descriptor);

    return 0;

}

static unsigned int onstop(struct event_channel *channel)
{

    return 1;

}

void main(void)
{

    struct event_channel channel;

    event_initsignals(signals);
    event_setsignal(signals, EVENT_FILE, onfile);
    event_setsignal(signals, EVENT_STOP, onstop);

    while (event_listen(signals, &channel));

}

