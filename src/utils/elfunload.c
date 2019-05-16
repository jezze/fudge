#include <fudge.h>
#include <abi.h>

static void onfile(struct event_channel *channel)
{

    struct event_file *file = event_getdata(channel);

    file_open(file->descriptor);
    call_unload(file->descriptor);
    file_close(file->descriptor);

}

void main(void)
{

    struct event_channel channel;

    event_initsignals(&channel);
    event_setsignal(&channel, EVENT_FILE, onfile);
    event_listen(&channel);

}

