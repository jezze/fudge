#include <fudge.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int id = file_spawn(FILE_CP, "/bin/echo");

    if (id)
    {

        struct message_header header;
        char *path = "/data/motd.txt";
        char data[FUDGE_MSIZE];

        job_redirect(channel, id, EVENT_DATA, 1, channel->signals[EVENT_DATA].redirect);
        job_redirect(channel, id, EVENT_CLOSE, 2, 0);
        channel_place(channel, id, EVENT_FILE, ascii_length(path) + 1, path);
        channel_place(channel, id, EVENT_MAIN, 0, 0);
        channel_poll(channel, id, EVENT_CLOSE, &header, data);

    }

    channel_close(channel);

}

static void onredirect(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    channel_setredirect(channel, redirect->type, redirect->mode, redirect->id, source);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_MAIN, onmain);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen(&channel, 0, 0);

}

