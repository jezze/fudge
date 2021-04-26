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

    struct {struct message_header header; struct event_blockrequest blockrequest;} message;

    message.blockrequest.sector = request->blocksector;
    message.blockrequest.count = request->blockcount;

    message_initheader(&message.header, EVENT_BLOCKREQUEST, sizeof (struct event_blockrequest));
    file_writeall(FILE_G0, &message, message.header.length);

}

static unsigned int request_poll(struct request *request, struct channel *channel)
{

    struct message_header header;
    struct message_data data;

    while (channel_pollevent(channel, EVENT_DATA, &header, &data))
    {

        request->blockreads += buffer_write(blocks, BLOCKSIZE * 4, &data, message_datasize(&header), request->blockreads * BLOCKSIZE) / BLOCKSIZE;

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

    unsigned int length = ascii_lengthz(path);
    unsigned int offset = 0;

    while (sendpoll(request, channel, source, offset, sizeof (struct cpio_header) + 1024))
    {

        struct cpio_header *header = getdata(request);

        if (!cpio_validate(header))
            break;

        if (header->namesize == 11)
        {

            if (buffer_match("TRAILER!!!", header + 1, 11))
                break;

        }

        if (header->namesize == length)
        {

            if (buffer_match(path, header + 1, length))
                return request->offset;

        }

        offset = cpio_next(header, request->offset);

    }

    return ERROR;

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct request *request = &requests[0];
    unsigned int offset;

    file_link(FILE_G0);
    file_open(FILE_G0);

    offset = walk(channel, source, request, "build/data/help.txt");

    if (offset != ERROR)
    {

        if (sendpoll(request, channel, source, offset, sizeof (struct cpio_header) + 1024))
        {

            struct cpio_header *header = getdata(request);

            if (cpio_validate(header))
            {

                unsigned int count;

                if ((count = sendpoll(request, channel, source, offset + cpio_filedata(header), cpio_filesize(header))))
                    channel_reply(channel, EVENT_DATA, count, getdata(request));

            }

        }

    }

    file_close(FILE_G0);
    file_unlink(FILE_G0);
    channel_close(channel);

}

void init(struct channel *channel)
{

    if (!file_walk2(FILE_G0, "system:block/if:0/data"))
        return;

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

