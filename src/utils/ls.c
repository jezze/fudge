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

    channel_setsignal(channel, EVENT_MAIN, onmain2);

}

static void oninit(struct channel *channel)
{

    channel_setsignal(channel, EVENT_MAIN, onmain);
    channel_setsignal(channel, EVENT_FILE, onfile);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_listen(&channel, oninit);

}

