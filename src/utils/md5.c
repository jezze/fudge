#include <fudge.h>
#include <abi.h>

static struct md5 s;

static void ondone(struct channel *channel, void *mdata, unsigned int msize)
{

    unsigned int id = channel_reply(channel, EVENT_DATA);
    unsigned char digest[16];
    unsigned int i;

    md5_write(&s, digest);

    for (i = 0; i < 16; i++)
        channel_appendvalue(channel, digest[i], 16, 2);

    channel_appendstring(channel, "\n");
    channel_place(channel, id);
    channel_close(channel);

}

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    md5_read(&s, mdata, msize);

}

static void onfile(struct channel *channel, void *mdata, unsigned int msize)
{

    char buffer[FUDGE_BSIZE];
    unsigned int count;

    if (!file_walk2(FILE_L0, mdata))
        return;

    file_open(FILE_L0);

    while ((count = file_read(FILE_L0, buffer, FUDGE_BSIZE)))
        md5_read(&s, buffer, count);

    file_close(FILE_L0);

}

static void onredirect(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    channel_setredirect(channel, redirect->type, redirect->id);

}

void main(void)
{

    struct channel channel;

    md5_init(&s);
    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DONE, ondone);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen(&channel);

}

