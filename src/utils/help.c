#include <fudge.h>
#include <abi.h>

static unsigned int idecho;

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_sendfile(channel, idecho, "/data/help.txt");
    channel_sendmain(channel, idecho);
    channel_close(channel);

}

static void onredirect(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    if (redirect->mode == 2)
    {

        redirect->mode = 1;
        redirect->id = source;

    }

    channel_sendredirect(channel, idecho, redirect);

}

static void oninit(struct channel *channel)
{

    if (!file_walk2(FILE_CP, "/bin/echo"))
        return;

    idecho = call_spawn(FILE_CP);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_MAIN, onmain);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen2(&channel, oninit, 0);

}

