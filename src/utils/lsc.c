#include <fudge.h>
#include <abi.h>

static void list(struct channel *channel, unsigned int descriptor)
{

    struct record record;

    file_open(descriptor);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        char num[FUDGE_NSIZE];
        unsigned int id = channel_reply(channel, EVENT_DATA);

        channel_append(channel, ascii_wzerovalue(num, FUDGE_NSIZE, record.id, 16, 8, 0), num);
        channel_append(channel, 1, " ");
        channel_append(channel, ascii_wzerovalue(num, FUDGE_NSIZE, record.size, 16, 8, 0), num);
        channel_append(channel, 1, " ");
        channel_append(channel, record.length, record.name);
        channel_append(channel, 1, "\n");
        channel_place(channel, id);

        if (!file_step(descriptor))
            break;

    }

    file_close(descriptor);

}

static void onempty(struct channel *channel, void *mdata, unsigned int msize)
{

    list(channel, FILE_PW);

}

static void onfile(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_file *file = mdata;

    list(channel, file->descriptor);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_EMPTY, onempty);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_listen(&channel);

}

