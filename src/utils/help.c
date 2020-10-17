#include <fudge.h>
#include <abi.h>

static unsigned int idecho;

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    char *path = "/data/help.txt";

    channel_place3(channel, idecho, EVENT_FILE, ascii_length(path) + 1, path);
    channel_place3(channel, idecho, EVENT_MAIN, 0, 0);
    channel_close(channel);

}

static void onredirect(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    if (redirect->mode == 2)
        channel_sendredirect(channel, idecho, redirect->type, 1, source);
    else
        channel_sendredirect(channel, idecho, redirect->type, redirect->mode, redirect->id);

}

static void oninit(struct channel *channel)
{

    idecho = file_spawn(FILE_CP, "/bin/echo");

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_MAIN, onmain);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen2(&channel, oninit, 0);

}

