#include <fudge.h>
#include <abi.h>

static void list(struct channel *channel, unsigned int descriptor)
{

    struct record record;

    file_open(descriptor);
    channel_reply(channel, EVENT_DATA);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        if (event_avail(&channel->o) < record.length + 1)
        {

            channel_place(channel->o.header.target, &channel->o);
            event_reset(&channel->o);

        }

        event_append(&channel->o, record.length, record.name);
        event_append(&channel->o, 1, "\n");

        if (!file_step(descriptor))
            break;

    }

    channel_place(channel->o.header.target, &channel->o);
    file_close(descriptor);

}

static void onempty(struct channel *channel)
{

    list(channel, FILE_PW);

}

static void onfile(struct channel *channel)
{

    struct event_file *file = channel_getdata(channel);

    list(channel, file->descriptor);

}

void main(void)
{

    struct channel channel;

    channel_initsignals(&channel);
    channel_setsignal(&channel, EVENT_EMPTY, onempty);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_listen(&channel);

}

