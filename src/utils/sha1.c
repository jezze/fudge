#include <fudge.h>
#include <abi.h>

static struct sha1 s;

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    unsigned char digest[20];
    struct message_data data;
    unsigned int offset = 0;
    unsigned int i;

    sha1_write(&s, digest);

    for (i = 0; i < 20; i++)
        offset = message_appendvalue(&data, digest[i], 16, 2, offset);

    offset = message_appendstring(&data, "\n", offset);

    channel_place(channel, source, EVENT_DATA, offset, &data);
    channel_close(channel);

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    sha1_read(&s, mdata, msize);

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        char buffer[FUDGE_BSIZE];
        unsigned int count;

        file_open(FILE_L0);

        while ((count = file_read(FILE_L0, buffer, FUDGE_BSIZE)))
            sha1_read(&s, buffer, count);

        file_close(FILE_L0);

    }

}

void init(struct channel *channel)
{

    sha1_init(&s);
    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_DATA, ondata);
    channel_setcallback(channel, EVENT_FILE, onfile);

}

