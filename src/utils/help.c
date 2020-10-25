#include <fudge.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int id = file_spawn(FILE_CP, "/bin/echo");
    struct event_redirect redirect;
    char *path = "/data/help.txt";

    if (!id)
        return;

    redirect.type = EVENT_DATA;
    redirect.mode = 1;
    redirect.id = channel->signals[EVENT_DATA].redirect;

    channel_place(channel, id, EVENT_REDIRECT, sizeof (struct event_redirect), &redirect);
    channel_place(channel, id, EVENT_FILE, ascii_length(path) + 1, path);
    channel_place(channel, id, EVENT_MAIN, 0, 0);
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

