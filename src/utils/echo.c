#include <fudge.h>
#include <abi.h>

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    channel_reply(channel, EVENT_DATA);
    channel_append(channel, msize, mdata);
    channel_place(channel, channel->o.header.target);

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
        channel_append(channel, count, buffer);
        channel_place(channel, channel->o.header.target);

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

