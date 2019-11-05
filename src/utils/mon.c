#include <fudge.h>
#include <abi.h>

static unsigned int x;

static void sendrequest(struct channel *channel, unsigned int sector, unsigned int count)
{

    struct event_blockrequest blockrequest;

    blockrequest.sector = sector;
    blockrequest.count = count;

    channel_request(channel, EVENT_BLOCKREQUEST);
    channel_append(channel, sizeof (struct event_blockrequest), &blockrequest);
    channel_write(channel, FILE_G0);

}

static void ondone(struct channel *channel, void *mdata, unsigned int msize)
{

    sendrequest(channel, 0, 1);

}

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    struct cpio_header *header = mdata;

    if (cpio_validate(header))
    {

        unsigned int id = channel_reply(channel, EVENT_DATA);

        channel_append(channel, header->namesize - 1, header + 1);
        channel_appendstring(channel, "\n");
        channel_place(channel, id);

        if (++x < 8)
        {

            sendrequest(channel, 0, 1);

        }

        else
        {

            channel_close(channel);

        }

    }

}

static void onredirect(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    channel_setredirect(channel, redirect->type, redirect->id);

}

void main(void)
{

    struct channel channel;

    if (!file_walk2(FILE_G0, "/system/block/if:0/data"))
        return;

    file_open(FILE_G0);
    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DONE, ondone);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen(&channel);
    file_close(FILE_G0);

}

