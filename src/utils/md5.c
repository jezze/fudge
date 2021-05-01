#include <fudge.h>
#include <abi.h>

static struct md5 sum;

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    unsigned char digest[16];
    struct message_data data;
    unsigned int offset = 0;
    unsigned int i;

    md5_write(&sum, digest);

    for (i = 0; i < 16; i++)
        offset = message_putvalue(&data, digest[i], 16, 2, offset);

    offset = message_putstring(&data, "\n", offset);

    channel_reply(channel, EVENT_DATA, offset, &data);
    channel_close(channel);

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    md5_read(&sum, mdata, msize);

}

static void onpath(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        char buffer[BUFFER_SIZE];
        unsigned int count;

        file_open(FILE_L0);

        while ((count = file_read(FILE_L0, buffer, BUFFER_SIZE)))
            md5_read(&sum, buffer, count);

        file_close(FILE_L0);

    }

}

void init(struct channel *channel)
{

    md5_init(&sum);
    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_DATA, ondata);
    channel_setcallback(channel, EVENT_PATH, onpath);

}

