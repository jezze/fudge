#include <fudge.h>
#include <abi.h>

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_place(channel, source, EVENT_DATA, msize, mdata);

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        struct message_data data;
        unsigned int count;

        file_open(FILE_L0);

        while ((count = file_read(FILE_L0, &data, sizeof (struct message_data))))
            channel_place(channel, source, EVENT_DATA, count, &data);

        file_close(FILE_L0);

    }

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_DATA, ondata);
    channel_setcallback(channel, EVENT_FILE, onfile);

}

