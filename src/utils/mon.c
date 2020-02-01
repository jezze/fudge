#include <fudge.h>
#include <abi.h>

#define BLOCKSIZE 512
#define TYPE_FIND 1
#define TYPE_READ 2

struct request
{

    unsigned int type;
    unsigned int offset;
    unsigned int count;
    unsigned int blocksector;
    unsigned int blockoffset;
    unsigned int blockcount;
    unsigned int blockreads;
    void *data;

} requests[8];

static char blocks[BLOCKSIZE * 4];
static unsigned int qp;

static void request_notifydata(struct request *request, void *data, unsigned int size)
{

    unsigned int count = memory_write(request->data, BLOCKSIZE * 4, data, size, request->blockreads * BLOCKSIZE);

    request->blockreads += count / BLOCKSIZE;

}

static void request_init(struct request *request, unsigned int type, unsigned int offset, unsigned int count)
{

    request->type = type;
    request->offset = offset;
    request->count = count;
    request->blocksector = offset / BLOCKSIZE;
    request->blockoffset = offset % BLOCKSIZE;
    request->blockcount = ((BLOCKSIZE - 1) + request->blockoffset + request->count) / BLOCKSIZE;
    request->blockreads = 0;
    request->data = blocks;

}

static void sendblockrequest(struct channel *channel, unsigned int sector, unsigned int count)
{

    struct event_blockrequest blockrequest;

    blockrequest.sector = sector;
    blockrequest.count = count;

    channel_request(channel, EVENT_BLOCKREQUEST);
    channel_append(channel, sizeof (struct event_blockrequest), &blockrequest);
    channel_write(channel, FILE_G0);

}

static void handlefind(struct channel *channel, unsigned int source, struct request *request)
{

    struct cpio_header *header = (struct cpio_header *)((char *)request->data + request->blockoffset);

    if (cpio_validate(header))
    {

        /* need to check if namesize is too big, otherwise request more data */
        /* this way we can remove the + BLOCKSIZE at the end of request_init calls */

        channel_request(channel, EVENT_DATA);
        channel_append(channel, header->namesize - 1, header + 1);
        channel_appendstring(channel, "\n");
        channel_place(channel, source);

        /* request next entry */
        request_init(request, TYPE_FIND, cpio_next(header, request->offset), sizeof (struct cpio_header) + BLOCKSIZE);
        sendblockrequest(channel, request->blocksector, request->blockcount);

    }

    else
    {

        channel_close(channel);

    }

}

static void handleread(struct channel *channel, unsigned int source, struct request *request)
{

    channel_close(channel);

}

static void ondone(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct request *request = &requests[qp];

    /* request first entry */
    request_init(request, TYPE_FIND, 0, sizeof (struct cpio_header) + BLOCKSIZE);
    sendblockrequest(channel, request->blocksector, request->blockcount);

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct request *request = &requests[qp];

    request_notifydata(request, mdata, msize);

    if (request->blockreads < request->blockcount)
        return;

    switch (request->type)
    {

    case TYPE_FIND:
        handlefind(channel, source, request);

        break;

    case TYPE_READ:
        handleread(channel, source, request);

        break;

    }

}

static void onredirect(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    channel_setredirect(channel, redirect->type, redirect->mode, redirect->id, source);

}

static void oninit(struct channel *channel)
{

    if (!file_walk2(FILE_G0, "/system/block/if:0/data"))
        return;

    file_open(FILE_G0);

}

static void onexit(struct channel *channel)
{

    file_close(FILE_G0);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DONE, ondone);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen2(&channel, oninit, onexit);

}

