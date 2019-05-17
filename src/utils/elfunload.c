#include <fudge.h>
#include <abi.h>

static void onfile(struct channel *channel)
{

    struct event_file *file = channel_getdata(channel);

    file_open(file->descriptor);
    call_unload(file->descriptor);
    file_close(file->descriptor);

}

void main(void)
{

    struct channel channel;

    channel_initsignals(&channel);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_listen(&channel);

}

