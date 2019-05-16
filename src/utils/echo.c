#include <fudge.h>
#include <abi.h>

static unsigned int ondata(struct event_channel *channel)
{

    event_reply(channel, EVENT_DATA);
    event_append(&channel->o, event_getdatasize(channel), event_getdata(channel));
    event_place(channel->o.header.target, &channel->o);

    return 0;

}

static unsigned int onfile(struct event_channel *channel)
{

    struct event_file *file = event_getdata(channel);
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE - sizeof (struct event_header))))
    {

        event_reply(channel, EVENT_DATA);
        event_append(&channel->o, count, buffer);
        event_place(channel->o.header.target, &channel->o);

    }

    file_close(file->descriptor);

    return 0;

}

void main(void)
{

    struct event_channel channel;

    event_initsignals(&channel);
    event_setsignal(&channel, EVENT_DATA, ondata);
    event_setsignal(&channel, EVENT_FILE, onfile);
    event_listen(&channel);

}

