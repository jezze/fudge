#include <fudge.h>
#include <abi.h>

void main(void)
{

    struct channel channel;
    struct event_file file;
    unsigned int id;

    channel_init(&channel);

    file.descriptor = FILE_P0;

    if (!file_walk2(FILE_CP, "/bin/echo"))
        return;

    if (!file_walk2(FILE_C0, "/data/help.txt"))
        return;

    id = call_spawn();

    if (!id)
        return;

    channel_listen(&channel);
    channel_forward(&channel, EVENT_FILE);
    channel_append(&channel, sizeof (struct event_file), &file);
    channel_place(&channel, id);
    channel_forward(&channel, EVENT_STOP);
    channel_place(&channel, id);

}

