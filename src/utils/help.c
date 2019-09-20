#include <fudge.h>
#include <abi.h>

static void onstop(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_file file;
    unsigned int id;

    file.descriptor = FILE_P0;

    if (!file_walk2(FILE_CP, "/bin/echo"))
        return;

    if (!file_walk2(FILE_C0, "/data/help.txt"))
        return;

    id = call_spawn();

    if (!id)
        return;

    channel_forward(channel, EVENT_FILE);
    channel_append(channel, sizeof (struct event_file), &file);
    channel_place(channel, id);
    channel_forward(channel, EVENT_STOP);
    channel_place(channel, id);
    channel_exit(channel);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_STOP, onstop);
    channel_listen(&channel);

}

