#include <fudge.h>
#include <abi.h>

static struct crc sum;

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_data data;
    unsigned int offset = 0;

    offset = message_putvalue(&data, crc_finalize(&sum), 10, 0, offset);
    offset = message_putstring(&data, "\n", offset);

    channel_place(channel, EVENT_DATA, offset, &data);
    channel_close(channel);

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    crc_read(&sum, mdata, msize);

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        char buffer[BUFFER_SIZE];
        unsigned int count;

        file_open(FILE_L0);

        while ((count = file_read(FILE_L0, buffer, BUFFER_SIZE)))
            crc_read(&sum, buffer, count);

        file_close(FILE_L0);

    }

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_DATA, ondata);
    channel_setcallback(channel, EVENT_FILE, onfile);

}

