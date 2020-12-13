#include <fudge.h>
#include <abi.h>

static void print(struct channel *channel, unsigned int source, struct record *record, char *name, unsigned int length)
{

    struct message_data data;
    unsigned int offset = 0;

    if (record->length < length || !buffer_match(record->name, name, length))
        return;

    offset = message_putbuffer(&data, record->length, record->name, offset);
    offset = message_putstring(&data, "\n", offset);

    channel_place(channel, EVENT_DATA, offset, &data);

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    file_open(FILE_PW);

    do
    {

        struct record record;

        if (file_readall(FILE_PW, &record, sizeof (struct record)))
            print(channel, source, &record, mdata, msize);

    } while (file_step(FILE_PW));

    file_close(FILE_PW);

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_DATA, ondata);

}

