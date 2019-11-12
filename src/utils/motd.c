#include <fudge.h>
#include <abi.h>

static void ondone(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_close(channel);

}

static void onempty(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_redirect redirect;
    unsigned int id;

    if (!file_walk2(FILE_CP, "/bin/echo"))
        return;

    id = call_spawn(FILE_CP);

    if (!id)
        return;

    redirect.mode = 1;
    redirect.id = channel->signals[EVENT_DATA].redirect;
    redirect.type = EVENT_DATA;

    channel_request(channel, EVENT_REDIRECT);
    channel_append(channel, sizeof (struct event_redirect), &redirect);
    channel_place(channel, id);
    channel_request(channel, EVENT_FILE);
    channel_append(channel, 15, "/data/motd.txt");
    channel_place(channel, id);
    channel_request(channel, EVENT_DONE);
    channel_place(channel, id);

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
    channel_setsignal(&channel, EVENT_DONE, ondone);
    channel_setsignal(&channel, EVENT_EMPTY, onempty);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen(&channel);

}

