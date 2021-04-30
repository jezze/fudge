#include <fudge.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    file_open(FILE_G0);

    do
    {

        struct record record;

        if (file_readall(FILE_G0, &record, sizeof (struct record)))
        {

            struct message_data data;
            unsigned int offset = 0;

            offset = message_putbuffer(&data, record.length, record.name, offset);
            offset = message_putstring(&data, "\n", offset);

            channel_reply(channel, EVENT_DATA, offset, &data);

        }

    } while (file_step(FILE_G0));

    file_close(FILE_G0);
    channel_close(channel);

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    file_walk2(FILE_G0, mdata);

}

void init(struct channel *channel)
{

    file_duplicate(FILE_G0, FILE_PW);
    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_FILE, onfile);

}

