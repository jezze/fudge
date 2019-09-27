#include <fudge.h>
#include <abi.h>

struct request
{

    struct ipc_header origin;
    struct ipc_header header;
    struct event_blockrequest blockrequest;

};

static struct request requests[8];
static unsigned int x;

static void sendrequest(struct channel *channel, unsigned int sector, unsigned int count)
{

    struct request *request = &requests[0];

    ipc_create(&request->header, EVENT_BLOCKREQUEST, sizeof (struct event_blockrequest));

    request->blockrequest.sector = sector;
    request->blockrequest.count = count;

    file_writeall(FILE_G0, &request->header, request->header.length);

}

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    struct request *request = &requests[0];
    struct cpio_header *header = mdata;

    memory_copy(&channel->i, &request->origin, sizeof (struct ipc_header));

    if (cpio_validate(header))
    {

        unsigned int id = channel_reply(channel, EVENT_DATA);

        channel_append(channel, header->namesize, header + 1);
        channel_appendstring(channel, "\n");
        channel_place(channel, id);

        if (++x < 8)
        {

            sendrequest(channel, 0, 1);

        }

        else
        {

            x = 0;

        }

    }

}

/*
static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    struct request *request = &requests[0];
    unsigned short *data = mdata;
    unsigned int i;

    memory_copy(&channel->i, &request->origin, sizeof (struct ipc_header));

    for (i = 0; i < 64; i += 8)
    {

        unsigned int id = channel_reply(channel, EVENT_DATA);

        channel_appendvaluepadded(channel, i * 2, 16, 8);
        channel_appendstring(channel, " ");
        channel_appendvaluepadded(channel, data[i + 0], 16, 4);
        channel_appendstring(channel, " ");
        channel_appendvaluepadded(channel, data[i + 1], 16, 4);
        channel_appendstring(channel, " ");
        channel_appendvaluepadded(channel, data[i + 2], 16, 4);
        channel_appendstring(channel, " ");
        channel_appendvaluepadded(channel, data[i + 3], 16, 4);
        channel_appendstring(channel, " ");
        channel_appendvaluepadded(channel, data[i + 4], 16, 4);
        channel_appendstring(channel, " ");
        channel_appendvaluepadded(channel, data[i + 5], 16, 4);
        channel_appendstring(channel, " ");
        channel_appendvaluepadded(channel, data[i + 6], 16, 4);
        channel_appendstring(channel, " ");
        channel_appendvaluepadded(channel, data[i + 7], 16, 4);
        channel_appendstring(channel, "\n");
        channel_place(channel, id);

    }

}
*/

static void onopen(struct channel *channel, void *mdata, unsigned int msize)
{

    struct request *request = &requests[0];

    memory_copy(&request->origin, &channel->i, sizeof (struct ipc_header));
    sendrequest(channel, 0, 1);

}

void main(void)
{

    struct channel channel;

    if (!file_walk2(FILE_G0, "/system/block/if:0/data"))
        return;

    file_open(FILE_G0);
    channel_init(&channel);
    channel_setsignal(&channel, EVENT_OPEN, onopen);
    channel_nosignal(&channel, EVENT_CLOSE);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_listen(&channel);
    file_close(FILE_G0);

}

