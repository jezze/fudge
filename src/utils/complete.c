#include <fudge.h>
#include <abi.h>

static void complete(struct channel *channel, unsigned int source, unsigned int descriptor, void *name, unsigned int length)
{

    struct record record;

    file_open(descriptor);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        if (record.length >= length && memory_match(record.name, name, length))
        {

            struct message_data data;
            unsigned int offset = 0;

            offset = message_append(&data, offset, record.length, record.name);
            offset = message_appendstring(&data, "\n", offset);

            channel_place(channel, source, EVENT_DATA, offset, &data);

        }

        if (!file_step(descriptor))
            break;

    }

    file_close(descriptor);

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    complete(channel, source, FILE_PW, mdata, msize);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_listen(&channel, 0);

}

