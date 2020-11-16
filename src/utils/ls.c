#include <fudge.h>
#include <abi.h>

static void print(struct channel *channel, unsigned int source, struct record *record)
{

    struct message_data data;
    unsigned int offset = 0;

    offset = message_append(&data, offset, record->length, record->name);
    offset = message_appendstring(&data, "\n", offset);

    channel_place(channel, source, EVENT_DATA, offset, &data);

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    file_open(FILE_PW);

    do
    {

        struct record record;

        if (file_readall(FILE_PW, &record, sizeof (struct record)))
            print(channel, source, &record);

    } while (file_step(FILE_PW));

    file_close(FILE_PW);
    channel_close(channel);

}

static void onmain2(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_close(channel);

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_G0, mdata))
    {

        file_open(FILE_G0);

        do
        {

            struct record record;

            if (file_readall(FILE_G0, &record, sizeof (struct record)))
                print(channel, source, &record);

        } while (file_step(FILE_G0));

        file_close(FILE_G0);

    }

    channel_setcallback(channel, EVENT_MAIN, onmain2);

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_FILE, onfile);

}

