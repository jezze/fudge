#include <fudge.h>
#include <abi.h>

static struct sha1 s;

static void ondone(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    unsigned char digest[20];
    unsigned int i;

    sha1_write(&s, digest);
    channel_request(channel, EVENT_DATA);

    for (i = 0; i < 20; i++)
        channel_appendvalue(channel, digest[i], 16, 2);

    channel_appendstring(channel, "\n");
    channel_place(channel, source);
    channel_close(channel);

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    sha1_read(&s, mdata, msize);

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    char buffer[FUDGE_BSIZE];
    unsigned int count;

    if (!file_walk2(FILE_L0, mdata))
        return;

    file_open(FILE_L0);

    while ((count = file_read(FILE_L0, buffer, FUDGE_BSIZE)))
        sha1_read(&s, buffer, count);

    file_close(FILE_L0);

}

static void onredirect(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    channel_setredirect(channel, redirect->type, redirect->id, source);

}

void main(void)
{

    struct channel channel;

    sha1_init(&s);
    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DONE, ondone);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen(&channel);

}

