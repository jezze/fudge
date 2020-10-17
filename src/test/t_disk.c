#include <fudge.h>
#include <abi.h>

#define BLOCKSIZE 512
#define ERROR 0xFFFFFFFF

struct request
{

    unsigned int source;
    unsigned int offset;
    unsigned int count;
    unsigned int blocksector;
    unsigned int blockoffset;
    unsigned int blockcount;
    unsigned int blockreads;

} requests[8];

static char blocks[BLOCKSIZE * 4];

static void request_init(struct request *request, unsigned int source, unsigned int offset, unsigned int count)
{

    request->source = source;
    request->offset = offset;
    request->count = count;
    request->blocksector = offset / BLOCKSIZE;
    request->blockoffset = offset % BLOCKSIZE;
    request->blockcount = ((BLOCKSIZE - 1) + request->blockoffset + request->count) / BLOCKSIZE;
    request->blockreads = 0;

}

static void request_send(struct request *request, struct channel *channel)
{

    struct event_blockrequest blockrequest;
    union message message;

    blockrequest.sector = request->blocksector;
    blockrequest.count = request->blockcount;

    channel_header(&message, EVENT_BLOCKREQUEST);
    channel_append(&message, sizeof (struct event_blockrequest), &blockrequest);
    channel_write(channel, &message, FILE_G0);

}

static unsigned int request_poll(struct request *request, struct channel *channel)
{

    struct ipc_header header;
    char data[FUDGE_BSIZE];

    while (channel_poll(channel, &header, data, EVENT_DATA))
    {

        unsigned int size = ipc_datasize(&header);
        unsigned int count = memory_write(blocks, BLOCKSIZE * 4, data, size, request->blockreads * BLOCKSIZE);

        request->blockreads += count / BLOCKSIZE;

        if (request->blockreads == request->blockcount)
            return request->count;

    }

    return 0;

}

static unsigned int sendpoll(struct request *request, struct channel *channel, unsigned int source, unsigned int offset, unsigned int count)
{

    request_init(request, source, offset, count);
    request_send(request, channel);

    return request_poll(request, channel);

}

static void *getdata(struct request *request)
{

    return ((char *)blocks + request->blockoffset);

}

static unsigned int walk(struct channel *channel, unsigned int source, struct request *request, char *path)
{

    unsigned int length = ascii_length(path) + 1;
    unsigned int offset = 0;

    while (sendpoll(request, channel, source, offset, sizeof (struct cpio_header) + 1024))
    {

        struct cpio_header *header = getdata(request);

        if (!cpio_validate(header))
            break;

        if (header->namesize == 11)
        {

            if (memory_match("TRAILER!!!", header + 1, 11))
                break;

        }

        if (header->namesize == length)
        {

            if (memory_match(path, header + 1, length))
                return request->offset;

        }

        offset = cpio_next(header, request->offset);

    }

    return ERROR;

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct request *request = &requests[0];
    unsigned int offset = walk(channel, source, request, "build/data/help.txt");
    unsigned int count;

    if (offset != ERROR)
    {

        if (sendpoll(request, channel, source, offset, sizeof (struct cpio_header) + 1024))
        {

            struct cpio_header *header = getdata(request);

            if (cpio_validate(header))
            {

                if ((count = sendpoll(request, channel, source, offset + cpio_filedata(header), cpio_filesize(header))))
                {

                    void *data = getdata(request);
                    union message message;

                    channel_header(&message, EVENT_DATA);
                    channel_append(&message, count, data);
                    channel_place(channel, &message, source);

                }

            }

        }

    }

    channel_close(channel);

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
    channel_setsignal(&channel, EVENT_MAIN, onmain);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen2(&channel, oninit, onexit);

}

