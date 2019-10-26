#include <fudge.h>
#include <abi.h>

static void ondone(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_file file;
    unsigned int id;

    if (!file_walk2(FILE_CP, "/bin/echo"))
        return;

    if (!file_walk2(FILE_C0, "/data/motd.txt"))
        return;

    id = call_spawn(FILE_CP);

    if (!id)
        return;

    file.descriptor = FILE_P0;

    channel_forward(channel, EVENT_FILE);
    channel_append(channel, sizeof (struct event_file), &file);
    channel_place(channel, id);
    channel_forward(channel, EVENT_DONE);
    channel_place(channel, id);
    channel_close(channel);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DONE, ondone);
    channel_listen(&channel);

}

