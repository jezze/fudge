#include <fudge.h>
#include <abi.h>

static void onclose(struct channel *channel, void *mdata, unsigned int msize)
{

    channel_close(channel);

}

static void onopen(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_file file;
    unsigned int id;

    file.descriptor = FILE_P0;

    if (!file_walk2(FILE_CP, "/bin/echo"))
        return;

    if (!file_walk2(FILE_C0, "/data/motd.txt"))
        return;

    id = call_spawn(FILE_CP);

    if (!id)
        return;

    channel_forward(channel, EVENT_OPEN);
    channel_place(channel, id);
    channel_forward(channel, EVENT_FILE);
    channel_append(channel, sizeof (struct event_file), &file);
    channel_place(channel, id);
    channel_forward(channel, EVENT_CLOSE);
    channel_place(channel, id);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_CLOSE, onclose);
    channel_setsignal(&channel, EVENT_OPEN, onopen);
    channel_listen(&channel);

}

