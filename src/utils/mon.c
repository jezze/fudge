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

    request->blockrequest.sector = sector;
    request->blockrequest.count = count;

    channel_request(channel, EVENT_BLOCKREQUEST);
    channel_append(channel, sizeof (struct event_blockrequest), &request->blockrequest);
    channel_write(channel, FILE_G0);

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

            channel_exit(channel);

        }

    }

}

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

