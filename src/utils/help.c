#include <fudge.h>
#include <abi.h>

static unsigned int idecho;

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_request(channel, EVENT_MAIN);
    channel_place(channel, idecho);
    channel_close(channel);

}

static void onempty(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_request(channel, EVENT_FILE);
    channel_append(channel, 15, "/data/help.txt");
    channel_place(channel, idecho);

}

static void onredirect(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    if (redirect->mode == 2)
    {

        redirect->mode = 1;
        redirect->id = source;

    }

    channel_request(channel, EVENT_REDIRECT);
    channel_append(channel, sizeof (struct event_redirect), redirect);
    channel_place(channel, idecho);

}

static void oninit(struct channel *channel)
{

    if (!file_walk2(FILE_CP, "/bin/echo"))
        return;

    idecho = call_spawn(FILE_CP);

}

static void onexit(struct channel *channel)
{

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_MAIN, onmain);
    channel_setsignal(&channel, EVENT_EMPTY, onempty);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen2(&channel, oninit, onexit);

}

