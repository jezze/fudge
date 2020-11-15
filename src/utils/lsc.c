#include <fudge.h>
#include <abi.h>

static void print(struct channel *channel, unsigned int source)
{

    file_open(FILE_G0);

    do
    {

        struct message_data data;
        unsigned int offset = 0;
        struct record record;

        file_readall(FILE_G0, &record, sizeof (struct record));

        offset = message_appendvalue(&data, record.id, 16, 8, offset);
        offset = message_appendstring(&data, " ", offset);
        offset = message_appendvalue(&data, record.size, 16, 8, offset);
        offset = message_appendstring(&data, " ", offset);
        offset = message_append(&data, offset, record.length, record.name);
        offset = message_appendstring(&data, "\n", offset);

        channel_place(channel, source, EVENT_DATA, offset, &data);

    } while (file_step(FILE_G0));

    file_close(FILE_G0);

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_duplicate(FILE_G0, FILE_PW))
        print(channel, source);

    channel_close(channel);

}

static void onmain2(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_close(channel);

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_G0, mdata))
        print(channel, source);

    channel_setcallback(channel, EVENT_MAIN, onmain2);

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_FILE, onfile);

}

