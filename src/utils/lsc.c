#include <fudge.h>
#include <abi.h>

static void list(struct channel *channel, unsigned int descriptor)
{

    struct record record;

    file_open(descriptor);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        unsigned int id = channel_reply(channel, EVENT_DATA);

        channel_appendvaluepadded(channel, record.id, 16, 8);
        channel_appendstring(channel, " ");
        channel_appendvaluepadded(channel, record.size, 16, 8);
        channel_appendstring(channel, " ");
        channel_append(channel, record.length, record.name);
        channel_appendstring(channel, "\n");
        channel_place(channel, id);

        if (!file_step(descriptor))
            break;

    }

    file_close(descriptor);

}

static void onempty(struct channel *channel, void *mdata, unsigned int msize)
{

    list(channel, FILE_PW);

}

static void onfile(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_file *file = mdata;

    list(channel, file->descriptor);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_EMPTY, onempty);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_listen(&channel);

}

