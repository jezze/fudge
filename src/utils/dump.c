#include <fudge.h>
#include <abi.h>

static void dump(struct channel *channel, unsigned int count, void *buffer)
{

    char *data = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int id = channel_reply(channel, EVENT_DATA);
        char num[FUDGE_NSIZE];

        channel_append(channel, ascii_wzerovalue(num, FUDGE_NSIZE, data[i], 16, 2, 0), num);
        channel_append(channel, 2, "  ");
        channel_place(channel, id);

    }

}

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    dump(channel, msize, mdata);

}

static void onfile(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_file *file = mdata;
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        dump(channel, count, buffer);

    file_close(file->descriptor);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_listen(&channel);

}

