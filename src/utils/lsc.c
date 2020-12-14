#include <fudge.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    file_open(FILE_PW);

    do
    {

        struct record record;

        if (file_readall(FILE_PW, &record, sizeof (struct record)))
        {

            struct message_data data;
            unsigned int offset = 0;

            offset = message_putvalue(&data, record.id, 16, 8, offset);
            offset = message_putstring(&data, " ", offset);
            offset = message_putvalue(&data, record.size, 16, 8, offset);
            offset = message_putstring(&data, " ", offset);
            offset = message_putbuffer(&data, record.length, record.name, offset);
            offset = message_putstring(&data, "\n", offset);

            channel_place(channel, EVENT_DATA, offset, &data);

        }

    } while (file_step(FILE_PW));

    file_close(FILE_PW);
    channel_close(channel);

}

static void ondirectory(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
        file_duplicate(FILE_PW, FILE_L0);

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_DIRECTORY, ondirectory);

}

