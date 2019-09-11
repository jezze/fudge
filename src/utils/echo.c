#include <fudge.h>
#include <abi.h>

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    channel_reply(channel, EVENT_DATA);
    event_append(&channel->o, msize, mdata);
    channel_place(channel->o.header.target, &channel->o);

}

static void onfile(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_file *file = mdata;
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE - sizeof (struct event_header))))
    {

        channel_reply(channel, EVENT_DATA);
        event_append(&channel->o, count, buffer);
        channel_place(channel->o.header.target, &channel->o);

    }

    file_close(file->descriptor);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_listen(&channel);

}

