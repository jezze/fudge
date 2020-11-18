#include <fudge.h>
#include <abi.h>

static struct crc s;

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_data data;
    unsigned int offset = 0;

    offset = message_appendvalue(&data, crc_finalize(&s), 10, 0, offset);
    offset = message_appendstring(&data, "\n", offset);

    channel_place(channel, source, EVENT_DATA, offset, &data);
    channel_close(channel, source);

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    crc_read(&s, mdata, msize);

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        char buffer[FUDGE_BSIZE];
        unsigned int count;

        file_open(FILE_L0);

        while ((count = file_read(FILE_L0, buffer, FUDGE_BSIZE)))
            crc_read(&s, buffer, count);

        file_close(FILE_L0);

    }

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_DATA, ondata);
    channel_setcallback(channel, EVENT_FILE, onfile);

}

