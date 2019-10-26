#include <fudge.h>
#include <abi.h>

static void complete(struct channel *channel, unsigned int descriptor, void *name, unsigned int length)
{

    struct record record;

    file_open(descriptor);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        if (record.length >= length && memory_match(record.name, name, length))
        {

            unsigned int id = channel_reply(channel, EVENT_DATA);

            channel_append(channel, record.length, record.name);
            channel_appendstring(channel, "\n");
            channel_place(channel, id);

        }

        if (!file_step(descriptor))
            break;

    }

    file_close(descriptor);

}

static void ondone(struct channel *channel, void *mdata, unsigned int msize)
{

    channel_close(channel);

}

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    complete(channel, FILE_PW, mdata, msize);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DONE, ondone);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_listen(&channel);

}

